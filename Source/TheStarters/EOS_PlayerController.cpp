// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_PlayerController.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/GameModeBase.h"
#include "EOS_GameSession.h"
#include "GameFramework/PlayerState.h"
#include "EOS_GameInstance.h"
#include "EOS_GameMode.h"

void AEOS_PlayerController::ServerSetDesiredPawnClass_Implementation(TSubclassOf<APawn> ChosenClass)
{
    if (!IsRunningDedicatedServer()) {
        UE_LOG(LogTemp, Error, TEXT("No authority return!"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    AEOS_GameMode* GM = Cast<AEOS_GameMode>(GetWorld()->GetAuthGameMode());
    if (!GM) return;

    AEOS_GameSession* EOS_GameSession = Cast<AEOS_GameSession>(GM->GameSession);
    if (!EOS_GameSession) return;

    FUniqueNetIdRepl PlayerId = this->PlayerState->GetUniqueId();

    if (!PlayerId.IsValid()) {
        UE_LOG(LogTemp, Error, TEXT("ServerSetDesiredPawnClass_Implementation: Invalid PlayerId"));
        return;
    }

    // Finally, store the class
    EOS_GameSession->SetPlayerDesiredClass(PlayerId, ChosenClass);

    UE_LOG(LogTemp, Log, TEXT("ServerSetDesiredPawnClass_Implementation: Server stored desired pawn class: %s, assosiated with id: %s"), *ChosenClass->GetName(), *PlayerId->ToString());
    // Respawn player, because it spawned with default pawn
    GetPawn()->Destroy();
    GM->RestartPlayer(this);
}

void AEOS_PlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Set input mode to Game Only
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);

    if (IsLocalController()) {
        UEOS_GameInstance* EOSGI = Cast<UEOS_GameInstance>(GetWorld()->GetGameInstance());
        if (!EOSGI) return;

        if (EOSGI->DesiredPawnClass) {
            ServerSetDesiredPawnClass(EOSGI->DesiredPawnClass);
        }
        else
            UE_LOG(LogTemp, Error, TEXT("AEOS_PlayerController::BeginPlay: DesiredPawnClass is not set in GameInstance!"));
    }
}
