// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_GameInstance.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

#define SEARCH_KEYWORDS TEXT("SearchKeywords")
#define SETTING_MAPNAME TEXT("MAPNAME")

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
	// For dedicated server
	/*sessionInfo.bAllowInvites = false;
	sessionInfo.bAllowJoinViaPresence = false;
	sessionInfo.bAllowJoinViaPresenceFriendsOnly = false;*/
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
	FName SearchKey = "KeyName";
	FString SearchValue = "KeyValue";

	IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = SubsystemRef->GetSessionInterface();
	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	// Remove the default search parameters that FOnlineSessionSearch sets up.
	SessionSearch->QuerySettings.SearchParams.Empty();

	SessionSearch->QuerySettings.Set(SearchKey, SearchValue, EOnlineComparisonOp::Equals); // Seach using our Key/Value pair

	// SessionSearch->MaxSearchResults = 20;
	// SessionSearch->bIsLanQuery = false;

	Session->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnFindSessionCompleted);

	UE_LOG(LogTemp, Log, TEXT("Finding session."));

	Session->FindSessions(0, SessionSearch.ToSharedRef());
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
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
	FString ConnectString;
	FOnlineSessionSearchResult* SessionToJoin;

	if (isSuccesful) {
		UE_LOG(LogTemp, Warning, TEXT("OnFindSessionCompleted, Num of session found: %d"), SessionSearch->SearchResults.Num());

		for (auto SessionInSearchResult : SessionSearch->SearchResults)
		{
			// Typically you want to check if the session is valid before joining. There is a bug in the EOS OSS where IsValid() returns false when the session is created on a DS. 
			// Instead of customizing the engine for this tutorial, we're simply not checking if the session is valid. The code below should go in this if statement once the bug is fixed. 
			/*
			if (SessionInSearchResult.IsValid())
			{


		}
			*/

			//Ensure the connection string is resolvable and store the info in ConnectInfo and in SessionToJoin
			if (Session->GetResolvedConnectString(SessionInSearchResult, NAME_GamePort, ConnectString))
			{
				SessionToJoin = &SessionInSearchResult;
				UE_LOG(LogTemp, Error, TEXT("??? %s"), *SessionToJoin->GetSessionIdStr());
			}

			UE_LOG(LogTemp, Error, TEXT("??? %s"), *ConnectString);

			break;
		}

		// Test Join for first avaible session
		if (SessionSearch->SearchResults.Num() > 0) {
			UE_LOG(LogTemp, Warning, TEXT("Joining session."));

			Session->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnJoinSessionCompleted);
			if (!Session->JoinSession(0, FName("SessionName"), SessionSearch->SearchResults[0]))
			{
				UE_LOG(LogTemp, Warning, TEXT("Join session failed"));
			}
		}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed to find session!!"));
		CreateEOSSession(false, false, 10);
	}
}

//void UEOS_GameInstance::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
//{
//	if (Result == EOnJoinSessionCompleteResult::Success) {
//		if (APlayerController* PlayerControllerRef = UGameplayStatics::GetPlayerController(GetWorld(), 0)) {
//			UE_LOG(LogTemp, Warning, TEXT("Joined session."));
//
//			// For the purposes of this tutorial overriding the ConnectString to point to localhost as we are testing locally. In a real game no need to override. Make sure you can connect over UDP to the ip:port of your server!
//			FString ConnectString = "127.0.0.1:7777";
//			FURL DedicatedServerURL(nullptr, *ConnectString, TRAVEL_Absolute);
//			FString DedicatedServerJoinError;
//			auto DedicatedServerJoinStatus = GEngine->Browse(GEngine->GetWorldContextFromWorldChecked(GetWorld()), DedicatedServerURL, DedicatedServerJoinError);
//			if (DedicatedServerJoinStatus == EBrowseReturnVal::Failure)
//			{
//				UE_LOG(LogTemp, Error, TEXT("Failed to browse for dedicated server. Error is: %s"), *DedicatedServerJoinError);
//			}
//
//			// To be thorough here you should modify your derived UGameInstance to handle the NetworkError and TravelError events. 
//			// As we are testing locally, and for the purposes of keeping this tutorial simple, this is omitted.
//
//			/*FString JoinAddress;
//
//			IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());
//			IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface();
//			SessionPtrRef->GetResolvedConnectString(SessionName, JoinAddress);
//
//			UE_LOG(LogTemp, Warning, TEXT("Join address is: %s"), *JoinAddress);
//
//			if (!JoinAddress.IsEmpty()) {
//				PlayerControllerRef->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
//			}*/
//		}
//	}
//}

void UEOS_GameInstance::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success) {
		FString MapName;
		
		// Отримуємо адресу сервера з сесії
		IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld());
		IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface();
		SessionPtrRef->GetSessionSettings(SessionName)->Get(SETTING_MAPNAME, MapName);
		

		// JoinAddress = "192.168.0.201";
		if (this->JoinAddress.IsEmpty()) {
			SessionPtrRef->GetResolvedConnectString(SessionName, this->JoinAddress);
		}
		
		UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionCompleted: Choosed join addres is: %s"), *JoinAddress);
		UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionCompleted: Map Name: %s"), *MapName);

		FURL ServerURL = FURL(nullptr, *JoinAddress, TRAVEL_Absolute);

		// Explicitly set map name (avoiding some weird bugs) 
		// !!! Requires setting up MAPNAME when creating session
		ServerURL.Map = MapName;
		FString ServerJoinError;

		UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionCompleted: Connection to: %s"), *ServerURL.ToString());

		auto ServerJoinStatus = GEngine->Browse(GEngine->GetWorldContextFromWorldChecked(GetWorld()), ServerURL, ServerJoinError);

		if (ServerJoinStatus == EBrowseReturnVal::Failure) {
			UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionCompleted: Failed to join server. Error: %s"), *ServerJoinError);

		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionCompleted: Successfully joined server: %s"), *ServerURL.ToString());
		}

	}
}

