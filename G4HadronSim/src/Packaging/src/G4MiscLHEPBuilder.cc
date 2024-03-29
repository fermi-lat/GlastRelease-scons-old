//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
#include "G4HadronSim/G4MiscLHEPBuilder.hh"

#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"

G4MiscLHEPBuilder::G4MiscLHEPBuilder(): wasActivated(false) {}
G4MiscLHEPBuilder::~G4MiscLHEPBuilder()
{
  if(wasActivated)
  {
    G4ProcessManager * aProcMan = 0;
    aProcMan = G4AntiProton::AntiProton()->GetProcessManager();
    if(aProcMan) aProcMan->RemoveProcess(&theAntiProtonInelastic);
    if(aProcMan) aProcMan->RemoveProcess(&theAntiProtonElasticProcess);
    aProcMan = G4AntiNeutron::AntiNeutron()->GetProcessManager();
    if(aProcMan) aProcMan->RemoveProcess(&theAntiNeutronInelastic);
    if(aProcMan) aProcMan->RemoveProcess(&theAntiNeutronElasticProcess);
    aProcMan = G4Lambda::Lambda()->GetProcessManager();
    if(aProcMan) aProcMan->RemoveProcess(&theLambdaInelastic);
    if(aProcMan) aProcMan->RemoveProcess(&theLambdaElasticProcess);
    aProcMan = G4AntiLambda::AntiLambda()->GetProcessManager();
    if(aProcMan) aProcMan->RemoveProcess(&theAntiLambdaInelastic);
    if(aProcMan) aProcMan->RemoveProcess(&theAntiLambdaElasticProcess);
    aProcMan = G4SigmaMinus::SigmaMinus()->GetProcessManager();
    if(aProcMan) aProcMan->RemoveProcess(&theSigmaMinusInelastic);
    if(aProcMan) aProcMan->RemoveProcess(&theSigmaMinusElasticProcess);
    aProcMan = G4AntiSigmaMinus::AntiSigmaMinus()->GetProcessManager();
    if(aProcMan) aProcMan->RemoveProcess(&theAntiSigmaMinusInelastic);
    if(aProcMan) aProcMan->RemoveProcess(&theAntiSigmaMinusElasticProcess);
    aProcMan = G4SigmaPlus::SigmaPlus()->GetProcessManager();
    if(aProcMan) aProcMan->RemoveProcess(&theSigmaPlusInelastic);
    if(aProcMan) aProcMan->RemoveProcess(&theSigmaPlusElasticProcess);
    aProcMan = G4AntiSigmaPlus::AntiSigmaPlus()->GetProcessManager();
    if(aProcMan) aProcMan->RemoveProcess(&theAntiSigmaPlusInelastic);
    if(aProcMan) aProcMan->RemoveProcess(&theAntiSigmaPlusElasticProcess);
    aProcMan = G4XiMinus::XiMinus()->GetProcessManager();
    if(aProcMan) aProcMan->RemoveProcess(&theXiMinusInelastic);
    if(aProcMan) aProcMan->RemoveProcess(&theXiMinusElasticProcess);
    aProcMan = G4AntiXiMinus::AntiXiMinus()->GetProcessManager();
    if(aProcMan) aProcMan->RemoveProcess(&theAntiXiMinusInelastic);
    if(aProcMan) aProcMan->RemoveProcess(&theAntiXiMinusElasticProcess);
    aProcMan = G4XiZero::XiZero()->GetProcessManager();
    if(aProcMan) aProcMan->RemoveProcess(&theXiZeroInelastic);
    if(aProcMan) aProcMan->RemoveProcess(&theXiZeroElasticProcess);
    aProcMan = G4AntiXiZero::AntiXiZero()->GetProcessManager();
    if(aProcMan) aProcMan->RemoveProcess(&theAntiXiZeroInelastic);
    if(aProcMan) aProcMan->RemoveProcess(&theAntiXiZeroElasticProcess);
    aProcMan = G4OmegaMinus::OmegaMinus()->GetProcessManager();
    if(aProcMan) aProcMan->RemoveProcess(&theOmegaMinusInelastic);
    if(aProcMan) aProcMan->RemoveProcess(&theOmegaMinusElasticProcess);
    aProcMan = G4AntiOmegaMinus::AntiOmegaMinus()->GetProcessManager();
    if(aProcMan) aProcMan->RemoveProcess(&theAntiOmegaMinusInelastic);
    if(aProcMan) aProcMan->RemoveProcess(&theAntiOmegaMinusElasticProcess);
  }
}

void G4MiscLHEPBuilder::Build()
{
  G4ProcessManager * aProcMan = 0;
  theElasticModel = new G4LElastic;
  wasActivated = true;
  
  // anti-Proton
  aProcMan = G4AntiProton::AntiProton()->GetProcessManager();
  theLEAntiProtonModel = new G4LEAntiProtonInelastic();
  theHEAntiProtonModel = new G4HEAntiProtonInelastic();
  theAntiProtonInelastic.RegisterMe(theLEAntiProtonModel);
  theAntiProtonInelastic.RegisterMe(theHEAntiProtonModel);
  aProcMan->AddDiscreteProcess(&theAntiProtonInelastic);
  theAntiProtonElasticProcess.RegisterMe(theElasticModel);
  aProcMan->AddDiscreteProcess(&theAntiProtonElasticProcess);

  // AntiNeutron
  aProcMan = G4AntiNeutron::AntiNeutron()->GetProcessManager();
  theLEAntiNeutronModel = new G4LEAntiNeutronInelastic();
  theHEAntiNeutronModel = new G4HEAntiNeutronInelastic();
  theAntiNeutronInelastic.RegisterMe(theLEAntiNeutronModel);
  theAntiNeutronInelastic.RegisterMe(theHEAntiNeutronModel);
  aProcMan->AddDiscreteProcess(&theAntiNeutronInelastic);
  theAntiNeutronElasticProcess.RegisterMe(theElasticModel);
  aProcMan->AddDiscreteProcess(&theAntiNeutronElasticProcess);

  // Lambda
  aProcMan = G4Lambda::Lambda()->GetProcessManager();
  theLELambdaModel = new G4LELambdaInelastic();
  theHELambdaModel = new G4HELambdaInelastic();
  theLambdaInelastic.RegisterMe(theLELambdaModel);
  theLambdaInelastic.RegisterMe(theHELambdaModel);
  aProcMan->AddDiscreteProcess(&theLambdaInelastic);
  theLambdaElasticProcess.RegisterMe(theElasticModel);
  aProcMan->AddDiscreteProcess(&theLambdaElasticProcess);
  
  // AntiLambda
  aProcMan = G4AntiLambda::AntiLambda()->GetProcessManager();
  theLEAntiLambdaModel = new G4LEAntiLambdaInelastic();
  theHEAntiLambdaModel = new G4HEAntiLambdaInelastic();
  theAntiLambdaInelastic.RegisterMe(theLEAntiLambdaModel);
  theAntiLambdaInelastic.RegisterMe(theHEAntiLambdaModel);
  aProcMan->AddDiscreteProcess(&theAntiLambdaInelastic);
  theAntiLambdaElasticProcess.RegisterMe(theElasticModel);
  aProcMan->AddDiscreteProcess(&theAntiLambdaElasticProcess);
    
  // SigmaMinus
  aProcMan = G4SigmaMinus::SigmaMinus()->GetProcessManager();
  theLESigmaMinusModel = new G4LESigmaMinusInelastic();
  theHESigmaMinusModel = new G4HESigmaMinusInelastic();
  theSigmaMinusInelastic.RegisterMe(theLESigmaMinusModel);
  theSigmaMinusInelastic.RegisterMe(theHESigmaMinusModel);
  aProcMan->AddDiscreteProcess(&theSigmaMinusInelastic);
  theSigmaMinusElasticProcess.RegisterMe(theElasticModel);
  aProcMan->AddDiscreteProcess(&theSigmaMinusElasticProcess);

  // anti-SigmaMinus
  aProcMan = G4AntiSigmaMinus::AntiSigmaMinus()->GetProcessManager();
  theLEAntiSigmaMinusModel = new G4LEAntiSigmaMinusInelastic();
  theHEAntiSigmaMinusModel = new G4HEAntiSigmaMinusInelastic();
  theAntiSigmaMinusInelastic.RegisterMe(theLEAntiSigmaMinusModel);
  theAntiSigmaMinusInelastic.RegisterMe(theHEAntiSigmaMinusModel);
  aProcMan->AddDiscreteProcess(&theAntiSigmaMinusInelastic);
  theAntiSigmaMinusElasticProcess.RegisterMe(theElasticModel);
  aProcMan->AddDiscreteProcess(&theAntiSigmaMinusElasticProcess);

  // SigmaPlus
  aProcMan = G4SigmaPlus::SigmaPlus()->GetProcessManager();
  theLESigmaPlusModel = new G4LESigmaPlusInelastic();
  theHESigmaPlusModel = new G4HESigmaPlusInelastic();
  theSigmaPlusInelastic.RegisterMe(theLESigmaPlusModel);
  theSigmaPlusInelastic.RegisterMe(theHESigmaPlusModel);
  aProcMan->AddDiscreteProcess(&theSigmaPlusInelastic);
  theSigmaPlusElasticProcess.RegisterMe(theElasticModel);
  aProcMan->AddDiscreteProcess(&theSigmaPlusElasticProcess);

  // anti-SigmaPlus
  aProcMan = G4AntiSigmaPlus::AntiSigmaPlus()->GetProcessManager();
  theLEAntiSigmaPlusModel = new G4LEAntiSigmaPlusInelastic();
  theHEAntiSigmaPlusModel = new G4HEAntiSigmaPlusInelastic();
  theAntiSigmaPlusInelastic.RegisterMe(theLEAntiSigmaPlusModel);
  theAntiSigmaPlusInelastic.RegisterMe(theHEAntiSigmaPlusModel);
  aProcMan->AddDiscreteProcess(&theAntiSigmaPlusInelastic);
  theAntiSigmaPlusElasticProcess.RegisterMe(theElasticModel);
  aProcMan->AddDiscreteProcess(&theAntiSigmaPlusElasticProcess);

  // XiMinus
  aProcMan = G4XiMinus::XiMinus()->GetProcessManager();
  theLEXiMinusModel = new G4LEXiMinusInelastic();
  theHEXiMinusModel = new G4HEXiMinusInelastic();
  theXiMinusInelastic.RegisterMe(theLEXiMinusModel);
  theXiMinusInelastic.RegisterMe(theHEXiMinusModel);
  aProcMan->AddDiscreteProcess(&theXiMinusInelastic);
  theXiMinusElasticProcess.RegisterMe(theElasticModel);
  aProcMan->AddDiscreteProcess(&theXiMinusElasticProcess);

  // anti-XiMinus
  aProcMan = G4AntiXiMinus::AntiXiMinus()->GetProcessManager();
  theLEAntiXiMinusModel = new G4LEAntiXiMinusInelastic();
  theHEAntiXiMinusModel = new G4HEAntiXiMinusInelastic();
  theAntiXiMinusInelastic.RegisterMe(theLEAntiXiMinusModel);
  theAntiXiMinusInelastic.RegisterMe(theHEAntiXiMinusModel);
  aProcMan->AddDiscreteProcess(&theAntiXiMinusInelastic);
  theAntiXiMinusElasticProcess.RegisterMe(theElasticModel);
  aProcMan->AddDiscreteProcess(&theAntiXiMinusElasticProcess);

  // XiZero
  aProcMan = G4XiZero::XiZero()->GetProcessManager();
  theLEXiZeroModel = new G4LEXiZeroInelastic();
  theHEXiZeroModel = new G4HEXiZeroInelastic();
  theXiZeroInelastic.RegisterMe(theLEXiZeroModel);
  theXiZeroInelastic.RegisterMe(theHEXiZeroModel);
  aProcMan->AddDiscreteProcess(&theXiZeroInelastic);
  theXiZeroElasticProcess.RegisterMe(theElasticModel);
  aProcMan->AddDiscreteProcess(&theXiZeroElasticProcess);

  // anti-XiZero
  aProcMan = G4AntiXiZero::AntiXiZero()->GetProcessManager();
  theLEAntiXiZeroModel = new G4LEAntiXiZeroInelastic();
  theHEAntiXiZeroModel = new G4HEAntiXiZeroInelastic();
  theAntiXiZeroInelastic.RegisterMe(theLEAntiXiZeroModel);
  theAntiXiZeroInelastic.RegisterMe(theHEAntiXiZeroModel);
  aProcMan->AddDiscreteProcess(&theAntiXiZeroInelastic);
  theAntiXiZeroElasticProcess.RegisterMe(theElasticModel);
  aProcMan->AddDiscreteProcess(&theAntiXiZeroElasticProcess);

  // OmegaMinus
  aProcMan = G4OmegaMinus::OmegaMinus()->GetProcessManager();
  theLEOmegaMinusModel = new G4LEOmegaMinusInelastic();
  theHEOmegaMinusModel = new G4HEOmegaMinusInelastic();
  theOmegaMinusInelastic.RegisterMe(theLEOmegaMinusModel);
  theOmegaMinusInelastic.RegisterMe(theHEOmegaMinusModel);
  aProcMan->AddDiscreteProcess(&theOmegaMinusInelastic);
  theOmegaMinusElasticProcess.RegisterMe(theElasticModel);
  aProcMan->AddDiscreteProcess(&theOmegaMinusElasticProcess);

  // anti-OmegaMinus
  aProcMan = G4AntiOmegaMinus::AntiOmegaMinus()->GetProcessManager();
  theLEAntiOmegaMinusModel = new G4LEAntiOmegaMinusInelastic();
  theHEAntiOmegaMinusModel = new G4HEAntiOmegaMinusInelastic();
  theAntiOmegaMinusInelastic.RegisterMe(theLEAntiOmegaMinusModel);
  theAntiOmegaMinusInelastic.RegisterMe(theHEAntiOmegaMinusModel);
  aProcMan->AddDiscreteProcess(&theAntiOmegaMinusInelastic);
  theAntiOmegaMinusElasticProcess.RegisterMe(theElasticModel);
  aProcMan->AddDiscreteProcess(&theAntiOmegaMinusElasticProcess);
}

// 2002 by J.P. Wellisch
