// Copyright Epic Games, Inc. All Rights Reserved.

#include "MenuSystemCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

/*
Below is delegate initialization before class constructor body
for the OnCreateSessionComplete() and OnFindSessionsComplete() functions
*/
AMenuSystemCharacter::AMenuSystemCharacter():
CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete))
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;


	GetCharacterMovement()->bOrientRotationToMovement = true; 	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; 
	CameraBoom->bUsePawnControlRotation = true; 

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false; 

	//To initialize the OnlineSubsystem, the Get() function is used - which returns a pointer
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				20.f,
				FColor::Red,
				FString::Printf(TEXT("Found subsystem %s"),
				*OnlineSubsystem->GetSubsystemName().ToString())
				);
		}
	}
}

void AMenuSystemCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AMenuSystemCharacter::CreateGameSession()
{
	/*
		Pointer validity check
	*/
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}
	/*
		Checking if the session already exists,
		and if the existing session is not nullptr then it is destroyed.
	*/
	auto ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		OnlineSessionInterface->DestroySession(NAME_GameSession);
	}
	/*
		Adding a delegate to the list of delegates before creating a session
	*/
	OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	/*
		
		To create a new object of type TSharedPtr, use the MakeShareable function	
		
	*/
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = false; 
	SessionSettings->NumPublicConnections = 4; // number of connections 
	SessionSettings->bAllowJoinInProgress = true; // is it possible to connect if the session has already been started 
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bShouldAdvertise = true; // session will be displayed in the list of sessions
	SessionSettings->bUsesPresence = true; // allows using the player's region to connect
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->Set(FName("MatchType"), FString("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);//option to choose type of the session 

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();// get first player name for CreateSession function
	OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);
}

void AMenuSystemCharacter::JoinGameSession()//Finding a game session
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}

	OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	/*
		since the DefaultEngine.ini uses
		open steam dev id "SteamDevAppId=480",
		with a value of 10k there is a good chance to find connections
	*/
	SessionSearch->MaxSearchResults = 10000; 
	SessionSearch->bIsLanQuery = false;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
}

void AMenuSystemCharacter::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Blue,
				FString::Printf(TEXT("Created session: %s"), *SessionName.ToString())
			);
		}
		/*
			if bWasSuccessful == true, we travel to Lobby map and open it as a listen server 
		*/
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(FString("/Game/ThirdPerson/Maps/Lobby?listen")); // "?listen" - open level as a Listen server
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Failed to create session!"))
			);
		}
	}
}

void AMenuSystemCharacter::OnFindSessionsComplete(bool bWasSuccessful)
{
	/*
	https://docs.unrealengine.com/5.1/en-US/API/Plugins/OnlineSubsystem/FOnlineSessionSearch/SearchResults/
	*/

	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}

	for (auto Result : SessionSearch->SearchResults) // looping through and checking ID and Username
	{
		FString Id = Result.GetSessionIdStr();
		FString User = Result.Session.OwningUserName;
		FString MatchType;
		
		/*
			below we are cheking the match type to make sure we have correct one 
		*/
		Result.Session.SessionSettings.Get(FName("MatchType"), MatchType); 
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Cyan,
				FString::Printf(TEXT("Id: %s, User: %s"), *Id, *User)
			);
		}
		/*
			cheking the MatchType data value 
		*/
		if (MatchType == FString("FreeForAll"))
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Cyan,
					FString::Printf(TEXT("Joining Match Type: %s"), *MatchType)
				);
			}
			/*
				after we are found out a corect type match, we can join it.
				we are joining to the session by using another delegate JoinSessionCompleteDelegate
			*/
			OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Result);
		}

	}
}

void AMenuSystemCharacter::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}
	FString Address;
	if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString::Printf(TEXT("Connect string: %s"), *Address)
			);
		}

		APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (PlayerController)
		{
			PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
		}
	}
}

void AMenuSystemCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMenuSystemCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMenuSystemCharacter::Look);
	}
}

void AMenuSystemCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMenuSystemCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}




