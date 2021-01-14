#include "G4NRFPhysics.hh"

#include "G4RayleighScattering.hh"
#include "G4LivermoreRayleighModel.hh"

G4NRFPhysics::G4NRFPhysics(const G4String &name, G4bool use_xsec_tables_in,
  G4bool use_xsec_integration_in, G4bool force_isotropic_in, G4bool standalone_in, G4bool Verbose_in)
  : use_xsec_tables(use_xsec_tables_in),
    use_xsec_integration(use_xsec_integration_in),
    force_isotropic(force_isotropic_in),
    standalone(standalone_in),
    Verbose(Verbose_in)
{
  G4LossTableManager::Instance();
  const G4String theName = name;
  SetPhysicsType(0);
}

G4NRFPhysics::~G4NRFPhysics()
{}

void G4NRFPhysics::ConstructParticle()
{}

void G4NRFPhysics::ConstructProcess() {
  G4NRF *nrf = new G4NRF("NRF", Verbose, use_xsec_tables, use_xsec_integration, force_isotropic, standalone);


// I edited this so that it works for Geant4 10.5
  auto aParticleIterator=GetParticleIterator();


  aParticleIterator->reset();
  while( (*aParticleIterator)() ) {
    G4ParticleDefinition       *particle = aParticleIterator->value();
    G4ProcessManager *particleProcessMgr = particle->GetProcessManager();
    G4String                particleName = particle->GetParticleName();
    if (particleName == "gamma") {
      particleProcessMgr->AddDiscreteProcess(nrf);
    }
  }
}
