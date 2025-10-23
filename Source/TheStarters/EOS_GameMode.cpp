// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_GameMode.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "EOS_PlayerController.h"
#include "EOS_GameSession.h"

AEOS_GameMode::AEOS_GameMode()
{
	UE_LOG(LogTemp, Warning, TEXT("AEOS_GameMode constructor!"));

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Characters/BP_BaseCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PlayerControllerClass = AEOS_PlayerController::StaticClass(); // Tutorial 2: Setting the PlayerController to our custom one.
	GameSessionClass = AEOS_GameSession::StaticClass(); // Tutorial 3: Setting the GameSession class to our custom one.

	// Tutorial 3: In a real game you may want to have a waiting room before sending players to the level. You can use seamless travel to do this and persist the EOS Session across levels. 
	// This is omitted in this tutorial to keep things simple. 
}


//void AEOS_GameMode::PostLogin(APlayerController* NewPlayer)
//{
//	Super::PostLogin(NewPlayer);
//
//	if (NewPlayer) {
//		FUniqueNetIdRepl uniqueNetIdRepl;
//
//		if (NewPlayer->IsLocalController()) {
//			ULocalPlayer* LocalPlayerRef = NewPlayer->GetLocalPlayer();
//
//			if (LocalPlayerRef) {
//				uniqueNetIdRepl = LocalPlayerRef->GetPreferredUniqueNetId();
//			}
//			else {
//				UNetConnection* RemoteNetConnectionRef = Cast<UNetConnection>(NewPlayer->Player);
//
//				check(IsValid(RemoteNetConnectionRef));
//
//				uniqueNetIdRepl = RemoteNetConnectionRef->PlayerId;
//			}
//		}
//		else {
//			UNetConnection* RemoteNetConnectionRef = Cast<UNetConnection>(NewPlayer->Player);
//
//			check(IsValid(RemoteNetConnectionRef));
//
//			uniqueNetIdRepl = RemoteNetConnectionRef->PlayerId;
//		}
//
//		TSharedPtr<const FUniqueNetId> UniqueNetId = uniqueNetIdRepl.GetUniqueNetId();
//
//		check(UniqueNetId != nullptr);
//
//		IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(NewPlayer->GetWorld());
//
//		IOnlineSessionPtr SessionRef = SubsystemRef->GetSessionInterface();
//
//		bool bRegistrationSuccess = SessionRef->RegisterPlayer(FName("MainSession"), *UniqueNetId, false);
//
//		if (bRegistrationSuccess) {
//			UE_LOG(LogTemp, Warning, TEXT("Registartion Successful"));
//		}
//	}
//}
