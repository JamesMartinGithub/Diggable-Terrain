// Copyright Epic Games, Inc. All Rights Reserved.

#include "MeshDeformGameMode.h"
#include "MeshDeformCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMeshDeformGameMode::AMeshDeformGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
