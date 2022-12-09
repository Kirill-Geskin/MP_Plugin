// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuSystemCharacter.generated.h"



UCLASS(config=Game)
class AMenuSystemCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

public:
	AMenuSystemCharacter();

protected:

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);	

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void BeginPlay();

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

public:

	/*
		Below is the pointer for the online interface session.
		IOnlineSessionPtr - is a smart pointer,
		and safe for multi-threaded environment.

		in order not to add an extra header,
		you can use a Shared pointer as a wrapper
	*/
	IOnlineSessionPtr OnlineSessionInterface;

protected:

	UFUNCTION(BlueprintCallable)
	void CreateGameSession(); // The function will be called when button 1 is pressed

	UFUNCTION(BlueprintCallable)
	void JoinGameSession(); // The function will be called when button 2 is pressed

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate; // delegat for function CreateGameSession()
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate; // delegat for function JoinGameSession()
	TSharedPtr<FOnlineSessionSearch>SessionSearch;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate; // delegat for the joining to the session in function OnFindSessionsComplete
};

