// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_GameMode.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

void AEOS_GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer) {
		FUniqueNetIdRepl uniqueNetIdRepl;

		if (NewPlayer->IsLocalController()) {
			ULocalPlayer* LocalPlayerRef = NewPlayer->GetLocalPlayer();

			if (LocalPlayerRef) {
				uniqueNetIdRepl = LocalPlayerRef->GetPreferredUniqueNetId();
			}
			else {
				UNetConnection* RemoteNetConnectionRef = Cast<UNetConnection>(NewPlayer->Player);

				check(IsValid(RemoteNetConnectionRef));

				uniqueNetIdRepl = RemoteNetConnectionRef->PlayerId;
			}
		}
		else {
			UNetConnection* RemoteNetConnectionRef = Cast<UNetConnection>(NewPlayer->Player);

			check(IsValid(RemoteNetConnectionRef));

			uniqueNetIdRepl = RemoteNetConnectionRef->PlayerId;
		}

		TSharedPtr<const FUniqueNetId> UniqueNetId = uniqueNetIdRepl.GetUniqueNetId();

		check(UniqueNetId != nullptr);

		IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(NewPlayer->GetWorld());

		IOnlineSessionPtr SessionRef = SubsystemRef->GetSessionInterface();

		bool bRegistrationSuccess = SessionRef->RegisterPlayer(FName("MainSession"), *UniqueNetId, false);

		if (bRegistrationSuccess) {
			UE_LOG(LogTemp, Warning, TEXT("Registartion Successful"));
		}
	}
}
