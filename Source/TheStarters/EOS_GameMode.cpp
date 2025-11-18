// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_GameMode.h"
#include "EOS_GameSession.h"
#include "GameFramework/PlayerState.h"
#include "EOS_PlayerController.h"

UClass* AEOS_GameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	AEOS_GameSession* Session = Cast<AEOS_GameSession>(GameSession);
	APlayerState* PS = InController ? InController->PlayerState : nullptr;

	if (Session && PS && PS->GetUniqueId().IsValid()) {
		TSubclassOf<APawn> ChosenClass = Session->GetPlayerDesiredClass(PS->GetUniqueId());
		if (ChosenClass)
		{
			DefaultPawnClass = ChosenClass;
			return ChosenClass;
		}
	}
	
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}