// Fill out your copyright notice in the Description page of Project Settings.

#include "HovercraftMovementComponent.h"
#include "Hovercraft.h"

void UHovercraftMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	UE_LOG(LogTemp, Warning, TEXT("RequestDirectMove called"));
	AHovercraft* Craft = Cast<AHovercraft>(GetOwner());
	if (!Craft)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not cast owner to AHovercraft in %s"), *GetName());
		return;
	}


	FVector HovercraftForward = GetOwner()->GetActorForwardVector().GetSafeNormal();

	FVector AIForwardIntention = MoveVelocity.GetSafeNormal();

	float ForwardThrow = FVector::DotProduct(HovercraftForward, AIForwardIntention);
	Craft->MoveForward(ForwardThrow);

	float RightThrow = FVector::CrossProduct(HovercraftForward, AIForwardIntention).Z;
	Craft->MoveRight(RightThrow);
}



