// Fill out your copyright notice in the Description page of Project Settings.

#include "HoverComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Classes/Engine/World.h"
#include "Hovercraft.h"


// Sets default values for this component's properties
UHoverComponent::UHoverComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHoverComponent::BeginPlay()
{
	Super::BeginPlay();

	//UE_LOG(LogTemp, Warning, TEXT("Owner: %s"), *GetOwner()->GetName());
	
}


// Called every frame
void UHoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bShouldHover)
	{
		// OldHover();
		Hover();
	}

	
}

void UHoverComponent::SetStaticMeshReference(UStaticMeshComponent * StaticMeshToSet)
{
	StaticMesh = StaticMeshToSet;
}

void UHoverComponent::OldHover()
{
	/* at the moment, no socket needed (thats why socket specific code is commented out)
	* sweep shape of cube to get accurate hit results
	* ignore cube itself so we can sweep directly from object itself
	* TODO : no idea why calculation is off by 0.25 cm
	*/


	//const FName TraceTag = FName("HoverTrace");
	//GetWorld()->DebugDrawTraceTag = TraceTag;

	FCollisionQueryParams CollisionParams = FCollisionQueryParams::DefaultQueryParam;
	//CollisionParams.TraceTag = TraceTag;
	CollisionParams.AddIgnoredActor(GetOwner());

	FCollisionShape CollisionShape = Cast<UStaticMeshComponent>(GetOwner()->GetRootComponent())->GetCollisionShape();

	FHitResult HitResult;

	if (!Cast<UStaticMeshComponent>(GetOwner()->GetRootComponent())) { return; }
	//FVector StartLocation = GetOwner()->GetActorLocation();
	FVector StartLocation = GetOwner()->GetActorLocation();//Cast<UStaticMeshComponent>(GetOwner()->GetRootComponent())->GetSocketLocation(FName("Ground"));
															//UE_LOG(LogTemp, Warning, TEXT("StartLocation: %s"), *StartLocation.ToString());
	FVector EndLocation = StartLocation - FVector(0, 0, 1) * HoverHeight;

	//if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, CollisionParams))
	if (GetWorld()->SweepSingleByChannel(HitResult, StartLocation, EndLocation, FQuat::Identity, ECollisionChannel::ECC_Visibility, CollisionShape, CollisionParams))
	{
		// check if hit location is within HoverHeight
		//UE_LOG(LogTemp, Warning, TEXT("Distance: %f"), HitResult.Distance/*+CollisionShape.Box.HalfExtentZ*/);

		float Ratio = HitResult.Distance / HoverHeight;		// used as alpha for linear interpolation

		FVector ForceToApply = FMath::Lerp<float>(HoverForceMaxStrength, 0.f, Ratio) * HitResult.ImpactNormal;

		Cast<UStaticMeshComponent>(GetOwner()->GetRootComponent())->AddForce(ForceToApply);

		/*FVector Force = FVector(0,0,1) * HoverForceMaxStrength;
		FVector ForceLocation = Cast<UStaticMeshComponent>(GetOwner()->GetRootComponent())->GetSocketLocation(FName("Ground"));
		Cast<UStaticMeshComponent>(GetOwner()->GetRootComponent())->AddForceAtLocation(Force, ForceLocation);*/
		UE_LOG(LogTemp, Warning, TEXT("Applying Force: %s"), *ForceToApply.ToString());


	}
}

void UHoverComponent::Hover()
{
	if (!StaticMesh) { return; }
	/* sweep shape of static mesh */


	// ignore actor itself
	FCollisionQueryParams CollisionParams = FCollisionQueryParams::DefaultQueryParam;
	CollisionParams.AddIgnoredActor(GetOwner());

	// get collision shape
	FCollisionShape CollisionShape = StaticMesh->GetCollisionShape();

	FHitResult HitResult;

	// Start location for sweep
	FVector StartLocation = StaticMesh->GetComponentLocation();
	FVector EndLocation = StartLocation - GetOwner()->GetActorUpVector().GetSafeNormal() * HoverHeight;

	/* debug information */
	//const FName TraceTag = FName("HoverTrace");
	//GetWorld()->DebugDrawTraceTag = TraceTag;
	//CollisionParams.TraceTag = TraceTag;



	if (GetWorld()->SweepSingleByChannel(HitResult, StartLocation, EndLocation, GetOwner()->GetActorRotation().Quaternion(), ECollisionChannel::ECC_Visibility, CollisionShape, CollisionParams))
	{
		AHovercraft* Craft = Cast<AHovercraft>(GetOwner());
		if (Craft)
		{
			// player character
			//FVector ForceToApply = HitResult.ImpactNormal * Craft->GetHovercraftMass() * 981;	//9.81 m/s²
			//StaticMesh->AddForce(ForceToApply);
			/*float Ratio = HitResult.Distance / HoverHeight;
			FVector ForceToApply = FMath::Lerp<float>(HoverForceMaxStrength, 0, Ratio) * HitResult.ImpactNormal;
			StaticMesh->AddForce(ForceToApply);*/
			FVector Velocity = GetOwner()->GetVelocity();
			Velocity.Z = 100;
			StaticMesh->SetPhysicsLinearVelocity(Velocity);
		}
	}

	// if collision
		// calculate distance
		// apply force to static mesh according to distance
}

