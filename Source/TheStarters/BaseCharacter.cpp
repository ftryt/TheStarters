ï»¿// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerState.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"

// Network
#include "Net/UnrealNetwork.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
    // Set this character to call Tick() every frame. You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // --- CAMERA ---
    // ÐšÐ°Ð¼ÐµÑ€Ð° Ð²Ñ–Ð´ Ð¿ÐµÑ€ÑˆÐ¾Ð³Ð¾ Ð»Ð¸Ñ†Ñ
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(RootComponent);
    FollowCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f)); // Eye level
    FollowCamera->bUsePawnControlRotation = true;

    // Movement does not affect rotation
    GetCharacterMovement()->bOrientRotationToMovement = false;

    // Body control from the controller
    bUseControllerRotationYaw = true;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    // Better character physics
    GetCharacterMovement()->GravityScale = 2.0f;
    GetCharacterMovement()->JumpZVelocity = 700.0f;
    GetCharacterMovement()->AirControl = 0.7f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
    GetCharacterMovement()->GroundFriction = 8.0f;
    GetCharacterMovement()->MaxAcceleration = 2048.0f;
    GetCharacterMovement()->Mass = 120.f;

    // Default socket name can also be set here if you like
    WeaponSocketName = TEXT("WeaponSocket");
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    // GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("BaseCharacter: V1.2 Initialized!"));

    ENetMode netMode = GetNetMode();

    FString netModeStr = "Net mode: ";

    switch (netMode)
    {
    case NM_Standalone:
        netModeStr += "NM_Standalone";
        break;
    case NM_DedicatedServer:
        netModeStr += "NM_DedicatedServer";
        break;
    case NM_ListenServer:
        netModeStr += "NM_ListenServer";
        break;
    case NM_Client:
        netModeStr += "NM_Client";
        break;
    case NM_MAX:
        netModeStr += "NM_MAX";
        break;
    default:
        break;
    }

    FString RoleStr;
    FString WorldName = GetWorld() ? GetWorld()->GetName() : "NoWorld";

    switch (GetLocalRole())
    {
    case ROLE_Authority:       RoleStr = "ROLE_Authority (Server)";        break;
    case ROLE_AutonomousProxy: RoleStr = "ROLE_AutonomousProxy (Local)";   break;
    case ROLE_SimulatedProxy:  RoleStr = "ROLE_SimulatedProxy (Other)";    break;
    default:                   RoleStr = "ROLE_Unknown";                   break;
    }

    FString DebugMsg = FString::Printf(TEXT("%s | %s | %s | %s"),
        *GetName(), *netModeStr, *RoleStr, *WorldName);

    /*GEngine->AddOnScreenDebugMessage(-1, 155555555.0f, FColor::Yellow, DebugMsg);
    GEngine->AddOnScreenDebugMessage(-1, 155555555.0f, FColor::Yellow, GetName());
    UE_LOG(LogTemp, Log, TEXT("%s"), *DebugMsg);*/

    // Change capsule color for local player
    if (GetName().Equals("BP_BaseCharacter_C_0") && GetCapsuleComponent())
    {
        UMaterialInstanceDynamic* DynMat = GetCapsuleComponent()->CreateAndSetMaterialInstanceDynamic(0);
        if (DynMat)
        {
            DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::Black);
        }

        GetCapsuleComponent()->ShapeColor = FColor::Black;
        GetCapsuleComponent()->MarkRenderStateDirty();
    }

    // Add Input Mapping Context
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(PlayerMappingContext, 0);
        }
    }

    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

    // Give this character its default weapon (based on DefaultWeaponRow)
    EquipDefaultWeapon();
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInput->BindAction(MoveAction,           ETriggerEvent::Triggered, this, &ABaseCharacter::Move);
        EnhancedInput->BindAction(SprintAction,         ETriggerEvent::Started,   this, &ABaseCharacter::StartSprint);
        EnhancedInput->BindAction(SprintAction,         ETriggerEvent::Completed, this, &ABaseCharacter::StopSprint);
        EnhancedInput->BindAction(SpecialAbilityAction, ETriggerEvent::Started,   this, &ABaseCharacter::SpecialAbility);
        EnhancedInput->BindAction(LookAction,           ETriggerEvent::Triggered, this, &ABaseCharacter::Look);
        EnhancedInput->BindAction(JumpAction,           ETriggerEvent::Started,   this, &ACharacter::Jump);
        EnhancedInput->BindAction(JumpAction,           ETriggerEvent::Completed, this, &ACharacter::StopJumping);
    }
}

void ABaseCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection,   MovementVector.X);
    }
}

void ABaseCharacter::Server_SetMoveSpeed_Implementation(float NewSpeed)
{
    MoveSpeed = NewSpeed;
    GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void ABaseCharacter::StartSprint(const FInputActionValue& Value)
{
    bIsSprinting = true;

    if (HasAuthority())
    {
        MoveSpeed = DefaultMoveSpeed * SprintMultiplier;
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }
    else
    {
        Server_SetMoveSpeed(DefaultMoveSpeed * SprintMultiplier);
    }
}

void ABaseCharacter::StopSprint(const FInputActionValue& Value)
{
    bIsSprinting = false;

    if (HasAuthority())
    {
        MoveSpeed = DefaultMoveSpeed;
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }
    else
    {
        Server_SetMoveSpeed(DefaultMoveSpeed);
    }
}

void ABaseCharacter::OnRep_MoveSpeed()
{
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ABaseCharacter::SpecialAbility(const FInputActionValue& Value)
{
    //UE_LOG(LogTemp, Log, TEXT("Special Ability triggered"));
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Special Ability triggered"));

    APlayerController* PC = GetWorld()->GetFirstPlayerController();

    if (PC && PC->PlayerState)
    {
        const FUniqueNetIdRepl& UniqueId = PC->PlayerState->GetUniqueId();
        if (UniqueId.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("SpecialAbility: Client UniqueNetId: %s"), *UniqueId->ToString());
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("SpecialAbility: Client UniqueNetId OK"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("SpecialAbility: Client UniqueNetId is INVALID."));
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("SpecialAbility: Client UniqueNetId is INVALID."));
        }
    }

    UEOS_GameInstance* GameInstanceRef = Cast<UEOS_GameInstance>(GetWorld()->GetGameInstance());

    if (!GameInstanceRef->CurrentSession.SessionResult.IsValid()) {
        UE_LOG(LogTemp, Error, TEXT("Session in GameInstance is invalid!"));
        return;
    }
    FName sessionName = FName(GameInstanceRef->CurrentSession.SessionResult.GetSessionIdStr());

    // LeaveSession(sessionName);
    LeaveSession(sessionName);

    if (PC)
    {
        PC->ClientTravel(TEXT("/Game/Maps/MainMenu"), TRAVEL_Absolute);
    }
}

void ABaseCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxis = Value.Get<FVector2D>();

    AddControllerYawInput(LookAxis.X);
    AddControllerPitchInput(LookAxis.Y * -1.f);
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABaseCharacter, MoveSpeed);
}

/* ================== WEAPONS ================== */

void ABaseCharacter::EquipDefaultWeapon()
{
    // If no row configured, do nothing
    if (!DefaultWeaponRow.DataTable || DefaultWeaponRow.RowName.IsNone())
    {
        return;
    }

    // Find the row in the DataTable (DT_WeaponList)
    const FST_WeaponTableRow* Row =
        DefaultWeaponRow.DataTable->FindRow<FST_WeaponTableRow>(
            DefaultWeaponRow.RowName,
            TEXT("EquipDefaultWeapon"));

    if (!Row)
    {
        return; // Invalid row name
    }

    // SpawnActor must be set in the row (TSubclassOf<AActor>)
    if (!Row->SpawnActor)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner      = this;
    SpawnParams.Instigator = this;

    // Spawn the weapon actor
    AActor* Weapon = World->SpawnActor<AActor>(
        Row->SpawnActor,
        GetActorLocation(),
        GetActorRotation(),
        SpawnParams);

    if (!Weapon)
    {
        return;
    }

    CurrentWeapon = Weapon;

    // Attach it to the character mesh at the specified socket
    Weapon->AttachToComponent(
        GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        WeaponSocketName);
}
