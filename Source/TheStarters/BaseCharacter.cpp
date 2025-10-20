// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// Network
#include "Net/UnrealNetwork.h"


// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // --- CAMERA ---
    //  амера в≥д першого лиц€
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

    // Better character physisc
    GetCharacterMovement()->GravityScale = 2.0f;             // сильн≥ше т€ж≥нн€ Ч персонаж падаЇ швидше 1.8Ц2.5
    GetCharacterMovement()->JumpZVelocity = 700.0f;          // сила стрибка 500Ц650
    GetCharacterMovement()->AirControl = 0.7f;               // керован≥сть у пов≥тр≥ 0.3Ц0.6
    GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f; // €к швидко гальмуЇ п≥сл€ руху 1500Ц2500
    GetCharacterMovement()->GroundFriction = 8.0f;           // терт€ з землею 6Ц10
    GetCharacterMovement()->MaxAcceleration = 2048.0f;       // €к швидко набираЇ швидк≥сть 1500Ц2500
    GetCharacterMovement()->Mass = 120.f;                    // дл€ в≥дчутт€ УвагиФ (не обов'€зково, але гарно впливаЇ) 

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
    case ROLE_Authority: RoleStr = "ROLE_Authority (Server)"; break;
    case ROLE_AutonomousProxy: RoleStr = "ROLE_AutonomousProxy (Local Player)"; break;
    case ROLE_SimulatedProxy: RoleStr = "ROLE_SimulatedProxy (Other Player)"; break;
    default: RoleStr = "ROLE_Unknown"; break;
    }

    FString DebugMsg = FString::Printf(TEXT("%s | %s | %s | %s"),
        *GetName(), *netModeStr, *RoleStr, *WorldName);

    /*GEngine->AddOnScreenDebugMessage(-1, 155555555.0f, FColor::Yellow, DebugMsg);
    GEngine->AddOnScreenDebugMessage(-1, 155555555.0f, FColor::Yellow, GetName());
    UE_LOG(LogTemp, Log, TEXT("%s"), *DebugMsg);*/

    // Change capsule color for local player
    if (GetName().Equals("BP_BaseCharacter_C_0") && GetCapsuleComponent())
    {
        // «м≥нюЇмо кол≥р капсули (матер≥ал)
        UMaterialInstanceDynamic* DynMat = GetCapsuleComponent()->CreateAndSetMaterialInstanceDynamic(0);
        if (DynMat)
        {
            DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::Black);
        }

        // јбо прост≥ше Ч зм≥нити в≥зуальний debug-кол≥р
        GetCapsuleComponent()->ShapeColor = FColor::Black;
        GetCapsuleComponent()->MarkRenderStateDirty(); // ќновити рендер
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
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Move);
        EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ABaseCharacter::StartSprint);
        EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABaseCharacter::StopSprint);
        EnhancedInput->BindAction(SpecialAbilityAction, ETriggerEvent::Started, this, &ABaseCharacter::SpecialAbility);
        EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Look);
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

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
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
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

