/////////////////////////////////////////////////////////////////////////////////
//
// name: MPIManager.cc
// date: 17 Sep 14
// auth: Zach Hartwig
// mail: hartwig@psfc.mit.edu
//
// desc: The MPIManager class handles most of the parallelization
//       functionality for custom builds of Geant4 simulations using
//       an instance of MPI (Open MPI, MPICH2, etc). It is constructed
//       as a meyer's singleton to provide a single instance of the
//       class that is available globally throughout the simulation
//       codebase (identical to the G4 manager concept). It is
//       responsible for such important functions as node
//       identification and control, event distribution, and data
//       reduction (i.e. aggregation of data contained on the slave
//       nodes to the master node for persistent output). In addition,
//       it provides a replacement method for launching particles for
//       parallel binaries that provides control of how the events are
//       distributed across the nodes as well as ability launch far
//       more events per run than the standard G4 /run/beamOn method.
//
//       Note that much of the MPIManager class code is protected by
//       compiler macros to enable non-MPI builds of the G4
//       simluation. Such macros can be used throughout the simulation
//       code to prevent compilation errors when MPI is not present on
//       the system.
//
/////////////////////////////////////////////////////////////////////////////////

#ifdef MANTIS_MPI_ENABLED

#include "G4RunManager.hh"

#include "mpi.h"

#include "limits.h"

#include "MPIManager.hh"
#include "MPIManagerMessenger.hh"

extern G4bool debug;
MPIManager *MPIManager::theMPImanager = 0;


MPIManager *MPIManager::GetInstance()
{ return theMPImanager; }


MPIManager::MPIManager(int argcMPI, char *argvMPI[])
{
  if(debug)
  {
    std::cout << "MPIManager::MPIManager -> Begin." << std::endl;
    std::cout << argcMPI << " Inputs: " << std::endl << argvMPI[0] << std::endl << argvMPI[1] << std::endl;
  }

  // Initialize the MPI execution environment.  The MPI::Init_thread
  // method is more specific than MPI::Init, allowing control of
  // thread level support.  Here, we use MPI::THREAD_SERIALIZED to
  // ensure that if multiple threads are present that only 1 thread
  // will make calls the MPI libraries at one time.
  MPI::Init(argcMPI, argvMPI);
  if(debug)
    std::cout << "MPIManager::MPIManager -> Init_thread complete!" << std::endl;
  // Get the size (number of) and the rank the present process
  size = MPI::COMM_WORLD.Get_size();
  rank = MPI::COMM_WORLD.Get_rank();
  if(debug)
    std::cout << "MPIManager::MPIManager -> Size and Rank: " << size << "\t" << rank << std::endl;
  // Set G4bools for master/slave identification
  isMaster = (rank == RANK_MASTER);
  isSlave = (rank != RANK_MASTER);

  // If the present process is a slave, redirect its std::output to
  // the slaveForum, a special directory for output. The output file
  // is the base specified by argvMPI[1] combined with the slave rank
  if(debug)
    std::cout << "Making Slave Ranks." << std::endl;

  if(isSlave){
    G4String fBase = argvMPI[1];
    std::ostringstream slaveRank;
    slaveRank << rank;
    G4String fName = fBase+slaveRank.str()+".out";

    // Redirect G4cout to the slave output file
    slaveOut.open(fName.c_str(), std::ios::out);
    G4cout.rdbuf(slaveOut.rdbuf());
  }

  // Initialize the number of events to be processed on all nodes
  totalEvents = 0;
  masterEvents = 0;
  slaveEvents = 0;

  // Initialize distribute events bool
  distributeEvents = true;

  // Create the seeds to ensure randomness in each node
  CreateSeeds();

  // Distribute the seeds to the nodes
  DistributeSeeds();

  if(theMPImanager)
    G4Exception("MPIManager::MPIManager()", "acroException009", FatalException, "The MPImanager was constructed twice!");
  theMPImanager = this;

  theMPImessenger = new MPIManagerMessenger(this);
}


MPIManager::~MPIManager()
{
  delete theMPImessenger;

  if(isSlave and slaveOut.is_open())
    slaveOut.close();

  // MPI::Finalize() terminates the MPI execution environment
  MPI::Finalize();
}


// An MPI specific method to run the beam (primary particles),
// allowing the user to specify the number of events to run, as well
// as whether or not to distribute those events evenly across the
// available nodes or to run the same number of events on all
// nodes. Note that "events" is of type G4double in order to handle
// a total number of events greater then 2.1e9 (range of G4int data type)
void MPIManager::BeamOn(G4double events, G4bool distribute)
{
  // Set distribute events bool
  distributeEvents = distribute;

  // Obtain the run mananager
  G4RunManager *runManager = G4RunManager::GetRunManager();

  // If set, distribute the total events to be processed evenly across
  // all available nodes, assigning possible "remainders" to master
  if(distributeEvents){

    slaveEvents = G4int(events/size);
    masterEvents = G4int(events-slaveEvents*(size-1));

    if(isMaster) {
      G4cout << "\nMANTIS ANNOUNCEMENT: # events in master = " << masterEvents
	     << " / # events in slave = "  << slaveEvents << "\n" << G4endl;
    }

    totalEvents = events;

    // Error check to ensure events < range_G4int
    if(masterEvents > 2e9 or slaveEvents > 2e9)
      ThrowEventError();

    if(isMaster)
      G4RunManager::GetRunManager()->BeamOn(masterEvents);
    else
      G4RunManager::GetRunManager()->BeamOn(slaveEvents);
  }

  // Otherwise, each node will run totalEvents number of events
  else {

    slaveEvents = G4int(events);
    masterEvents = G4int(events);

    if(isMaster)
      G4cout << "\nMANTIS ANNOUNCEMENT: # events in master = " << masterEvents
	     << " / # events in slave = "  << slaveEvents << G4endl;

    // Error check to ensure events < range_G4int
    if(events>2e9)
      ThrowEventError();

    // Store the total number of events on all nodes
    totalEvents = events*size;

     // Ruuuuuuuuuuuuuuuuun that baby!
    runManager->BeamOn(G4int(events));
  }
}


void MPIManager::ThrowEventError()
{
  // Because events variable must be passed to the G4RunManager,
  // which is expecting a G4int, unspecified behavior could result
  // if the events variable (which is a G4double and can exceed the
  // 2.1e9 data range of G4int) is actually passed. Therefore,
  // ensure that events is < 2.1e9 if the user has chosen NOT to
  // distribute events across the slaves
  G4cout << "\nMANTIS ANNOUNCMENT: You have chosen to run (n_events > n_G4int_range) on each of the \n"
	 <<   "                slaves! Since MPIManager passes n_events to the runManager\n"
	 <<   "                this could result in unspecified behavior. Please choose another\n"
	 <<   "                number of primary events or distribute them across nodes such that\n"
	 <<   "                (n_events < n_G4int_range = 2,147,483,647). MANTIS will now abort...\n"
	 << G4endl;

  G4Exception("MPIManager::ThrowEventError()","MANTISMPIManagerException001", FatalException, "MANTIS ANNOUNCEMENT: Crashing this ship like the Hindenburg!\n");
}


// Method to create randomized seeds for each node.  I currently store
// the seeds in vector in case I want them later on in the executable.
// I may also want to replicate runs, in which case a master seed that
// generates subsequent seeds should be used.

void MPIManager::CreateSeeds()
{
  // Create a list of seeds that will be distributed to all the nodes
  for(G4int i=0; i<size; i++){
    G4double x = G4UniformRand();
    G4long seed = G4long(x*LONG_MAX);
    seedPacket.push_back(seed);
  }

  // Check to ensure that no two seeds are alike
  G4bool doubleCount = false;
  while(doubleCount){
    for(G4int i=0; i<size; i++)
      for(G4int j=0; j<size; j++)

	// If two seeds are the same, create a new seed
	if( (i!=j) and (seedPacket[i] == seedPacket[j]) ){
	  G4double x = G4UniformRand();
	  seedPacket[j] = G4long(x*LONG_MAX);
	  doubleCount=true;
	}

    doubleCount = false;
  }
}


void MPIManager::DistributeSeeds()
{
  if(rank==0){
    for(size_t i=0; i<seedPacket.size(); i++)
      G4cout << "Rank[" << i << "] seed = " << seedPacket[i] << G4endl;
  }

  CLHEP::HepRandom::setTheSeed(seedPacket[rank]);
}


// MPI::COMM_WORLD.Barrier forces all nodes to reach a common point
// before proceeding. Useful to ensure that nodes are communicating as
// well as synchronized.
void MPIManager::ForceBarrier(G4String location)
{
  MPI::COMM_WORLD.Barrier();

  G4cout << "\nMANTIS ANNOUNCEMENT: All nodes have reached the MPI barrier at " << location << "!\n"
         <<   "                 Nodes now proceeding onward ... \n"
	 << G4endl;
}


// MPI::COMM_WORLD.Reduce reduces values on all nodes to a single
// value on the specified node by using an MPI predefined operation
// or a user specified operation.

G4double MPIManager::SumDoublesToMaster(G4double slaveValue)
{
  G4double masterSum = 0.;
  MPI::COMM_WORLD.Reduce(&slaveValue, &masterSum, 1, MPI::DOUBLE, MPI::SUM, 0);
  return masterSum;
}


G4int MPIManager::SumIntsToMaster(G4int slaveValue)
{
  G4int masterSum = 0;
  MPI::COMM_WORLD.Reduce(&slaveValue, &masterSum, 1, MPI::INT, MPI::SUM, 0);
  return masterSum;
}

#endif
