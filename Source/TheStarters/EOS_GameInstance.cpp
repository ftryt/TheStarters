// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_GameInstance.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

#define SEARCH_KEYWORDS TEXT("SearchKeywords")

void UEOS_GameInstance::LoginWithEOS(FString ID, FString Token, FString LoginType)
{
	// !!! Command line paramerts (-AUTH_TYPE ...) have priority over function parametrs

	IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());

	if (!SubsystemRef) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineSubsystem FAIL"));
		return;
	}

	IOnlineIdentityPtr Identity = SubsystemRef->GetIdentityInterface();

	if (!Identity) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineIdentityPtr FAIL"));
		return;
	}

	// If you're logged in, don't try to login again.
	// This can happen if your player travels to a dedicated server or different maps
	// Make this method availble to call from BeginPlay many times

	FUniqueNetIdPtr NetId = Identity->GetUniquePlayerId(0);

	if (NetId != nullptr && Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
	{
		return;
	}

	// Grab command line parameters. If empty call hardcoded login function - Hardcoded login function useful for Play In Editor. 
	FString AuthType;
	FParse::Value(FCommandLine::Get(), TEXT("AUTH_TYPE="), AuthType);
	UE_LOG(LogTemp, Log, TEXT("AuthType: "), *AuthType);
	// Add the callback function to handle login completion
	Identity->OnLoginCompleteDelegates->AddUObject(this, &UEOS_GameInstance::OnLoginEOSCompleted);

	// Initiate login process
	if (!AuthType.IsEmpty()) //If parameter is NOT empty we can autologin.
	{
		/*
		In most situations you will want to automatically log a player in using the parameters passed via CLI.
		For example, using the exchange code for the Epic Games Store.
		*/
		UE_LOG(LogTemp, Log, TEXT("AutoLogging into EOS...")); // Log to the UE logs that we are trying to log in. 

		if (!Identity->AutoLogin(0))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to login... ")); // Log to the UE logs that we are trying to log in.
		}
	}
	else
	{
		/*
		Fallback if the CLI parameters are empty. Useful for PIE.
		The type here could be developer if using the DevAuthTool, ExchangeCode if the game is launched via the Epic Games Launcher, etc...
		*/
		FOnlineAccountCredentials Credentials(LoginType, ID, Token);

		UE_LOG(LogTemp, Log, TEXT("Logging into EOS...")); // Log to the UE logs that we are trying to log in. 

		if (!Identity->Login(0, Credentials))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to login... ")); // Log to the UE logs that we are trying to log in.
		}
	}
}

FString UEOS_GameInstance::GetPlayerUsername()
{
	IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());
	FString userName = "Null";

	if (!SubsystemRef) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineSubsystem FAIL"));
		return userName;
	}

	IOnlineIdentityPtr IdentityInterface = SubsystemRef->GetIdentityInterface();

	if (!IdentityInterface) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineIdentityPtr FAIL"));
		return userName;
	}

	if (IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn) {
		userName = IdentityInterface->GetPlayerNickname(0);
	}

	return userName;
}

bool UEOS_GameInstance::IsPlayerLoggedIn()
{
	IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());
	bool isLogged = false;

	if (!SubsystemRef) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineSubsystem FAIL"));
		return isLogged;
	}

	IOnlineIdentityPtr IdentityInterface = SubsystemRef->GetIdentityInterface();

	if (!IdentityInterface) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineIdentityPtr FAIL"));
		return isLogged;
	}

	if (IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn) {
		isLogged = true;
	}

	return isLogged;
}

void UEOS_GameInstance::CreateEOSSession(bool isDedicatedServer, bool isLanServer, int32 numOfpublicConnections)
{
	IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());

	if (!SubsystemRef) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineSubsystem FAIL"));
		return;
	}

	IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface();

	if (!SessionPtrRef) {
		UE_LOG(LogTemp, Error, TEXT("GetSessionInterface FAIL"));
		return;
	}

	FOnlineSessionSettings sessionInfo;
	sessionInfo.bIsDedicated = isDedicatedServer;
	sessionInfo.bAllowInvites = true;
	sessionInfo.bIsLANMatch = isLanServer;
	sessionInfo.NumPublicConnections = numOfpublicConnections;
	sessionInfo.bUseLobbiesIfAvailable = true; //!!!
	sessionInfo.bUsesPresence = false;
	sessionInfo.bShouldAdvertise = true;
	sessionInfo.Set(SEARCH_KEYWORDS, FString("RandomHi"), EOnlineDataAdvertisementType::ViaOnlineService);

	SessionPtrRef->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnCreateSessionCompleted);
	SessionPtrRef->CreateSession(0, FName("MainSession"), sessionInfo);
}

void UEOS_GameInstance::DestroySession() {
	IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());

	if (!SubsystemRef) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineSubsystem FAIL"));
		return;
	}

	IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface();

	if (!SessionPtrRef) {
		UE_LOG(LogTemp, Error, TEXT("GetSessionInterface FAIL"));
		return;
	}

	SessionPtrRef->OnDestroySessionCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnDestroySessionCompleted);
	SessionPtrRef->DestroySession(FName("MainSession"));
}

void UEOS_GameInstance::FindSessionAndJoin()
{
	IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());

	if (!SubsystemRef) {
		UE_LOG(LogTemp, Error, TEXT("IOnlineSubsystem FAIL"));
		return;
	}

	IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface();

	if (!SessionPtrRef) {
		UE_LOG(LogTemp, Error, TEXT("GetSessionInterface FAIL"));
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	// Idk why but probably for searching only for lobby
	//SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	SessionSearch->MaxSearchResults = 20;
	// SessionSearch->bIsLanQuery = false;
	// SessionSearch->QuerySettings.SearchParams.Empty();
	SessionPtrRef->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnFindSessionCompleted);

	SessionPtrRef->FindSessions(0, SessionSearch.ToSharedRef());
}

void UEOS_GameInstance::JoinSession()
{

}

void UEOS_GameInstance::OnLoginEOSCompleted(int32 LocalUserNum, bool Success, const FUniqueNetId& UserId, const FString& Error)
{
	if (Success) {
		UE_LOG(LogTemp, Warning, TEXT("Login Success - UserID: %s"), *UserId.ToString());
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Login FAIL, Reason - %s"), *Error);
	}
}

void UEOS_GameInstance::OnCreateSessionCompleted(FName SessionName, bool isSuccesful)
{
	if (!isSuccesful) {
		UE_LOG(LogTemp, Error, TEXT("Cant create session: OnCreateSessionCompleted"));
		return;
	}

	GetWorld()->ServerTravel(OpenLevelText);
}

void UEOS_GameInstance::OnDestroySessionCompleted(FName SessionName, bool isSuccesful)
{
	if (!isSuccesful) {
		UE_LOG(LogTemp, Error, TEXT("Can't destroy session: OnDestroySessionCompleted"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Session - %s - destroyed"), *SessionName.ToString());
}

void UEOS_GameInstance::OnFindSessionCompleted(bool isSuccesful)
{
	if (isSuccesful) {
		UE_LOG(LogTemp, Warning, TEXT("OnFindSessionCompleted, session found"));

		IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());

		if (!SubsystemRef) {
			UE_LOG(LogTemp, Error, TEXT("IOnlineSubsystem FAIL"));
			return;
		}

		IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface();

		if (!SessionPtrRef) {
			UE_LOG(LogTemp, Error, TEXT("GetSessionInterface FAIL"));
			return;
		}

		if (SessionSearch->SearchResults.Num() > 0) {

			SessionPtrRef->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnJoinSessionCompleted);
			SessionPtrRef->JoinSession(0, FName("MainSession"), SessionSearch->SearchResults[0]);
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("No KURWA SearchResults = 0"));
			CreateEOSSession(false, false, 10);
		}

	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Could find session, creating new one"));
		CreateEOSSession(false, false, 10);
	}
}

void UEOS_GameInstance::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success) {
		if (APlayerController* PlayerControllerRef = UGameplayStatics::GetPlayerController(GetWorld(), 0)) {
			FString JoinAddress;

			IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());

			if (!SubsystemRef) {
				UE_LOG(LogTemp, Error, TEXT("IOnlineSubsystem FAIL"));
				return;
			}

			IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface();

			if (!SessionPtrRef) {
				UE_LOG(LogTemp, Error, TEXT("GetSessionInterface FAIL"));
				return;
			}

			SessionPtrRef->GetResolvedConnectString(FName("MainSession"), JoinAddress);

			UE_LOG(LogTemp, Warning, TEXT("Join address is: %s"), *JoinAddress);

			if (!JoinAddress.IsEmpty()) {
				PlayerControllerRef->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}
