// Fill out your copyright notice in the Description page of Project Settings.

#include "Hovercraft.h"
#include "Classes/Components/InputComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Components/PrimitiveComponent.h"
#include "Classes/Engine/World.h"
#include "HoverThruster.h"


// Sets default values
AHovercraft::AHovercraft()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AHovercraft::BeginPlay()
{
	Super::BeginPlay();

	// needs to be called before IsFallingArray.Init()
	InitializeThrusters();

	IsFallingArray.Init(false, NumberOfThrusters);
	
}

void AHovercraft::MoveForward(float Value)
{

	// TODO think about frame rate independency
	if (!StaticMesh) { return; }
	Value = FMath::Clamp<float>(Value, -1.f, 1.f);
	FVector ForceToApply = GetActorForwardVector() * ForwardAcceleration * Value;
	if (GetIsFalling())
	{
		ForceToApply *= ForwardAirborneControl;
	}

	StaticMesh->AddForce(ForceToApply);
}

void AHovercraft::MoveRight(float Value)
{
	// TODO think about frame rate independency
	if (!StaticMesh) { return; }
	Value = FMath::Clamp<float>(Value, -1.f, 1.f);
	FVector ForceToApply = GetActorRightVector() * SidewardAcceleration * Value;
	if (GetIsFalling())
	{
		ForceToApply *= SidewardsAirborneControl;
	}

	StaticMesh->AddForce(ForceToApply);
}

void AHovercraft::RotateRight(float Value)
{
	if (!StaticMesh || !GetWorld()) { return; }
	Value = FMath::Clamp<float>(Value, -1.f, 1.f);
	FRotator Rotator = FRotator(0, RotationSpeed * GetWorld()->DeltaTimeSeconds * Value, 0);
	StaticMesh->AddWorldRotation(Rotator);
}


void AHovercraft::SetStaticMeshReference(UStaticMeshComponent * MeshToSet)
{
	StaticMesh = MeshToSet;
}

void AHovercraft::CheckIsUpsideDown()
{
	if (!StaticMesh) { return; }
	// raytrace up
	FVector StartLocation = StaticMesh->GetComponentLocation();
	FVector EndLocation = StartLocation + StaticMesh->GetUpVector() * UpsideDownTraceLength;

	FCollisionQueryParams CollisionParams = FCollisionQueryParams::DefaultQueryParam;
	CollisionParams.AddIgnoredActor(this);

	FHitResult HitResult;

	/* Debug information */
	/*const FName TraceTag = FName("UpsideDownTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;*/
	const FName TraceTag = FName("HoverTrace");	// same name as in HoverThruster so we can draw both traces simultaneously
	CollisionParams.TraceTag = TraceTag;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, CollisionParams))
	{
		bIsUpsideDown = true;
	}
	else
	{
		bIsUpsideDown = false;
	}
}

void AHovercraft::InitializeThrusters()
{
	TArray<UActorComponent*> Thrusters = GetComponentsByClass(UHoverThruster::StaticClass());
	NumberOfThrusters = Thrusters.Num();
	for (const auto& Thruster : Thrusters)
	{
		HoverThrusters.Add(Cast<UHoverThruster>(Thruster));
	}
}

float AHovercraft::GetHovercraftMass()
{
	float Mass = 0.f;
	TArray<UPrimitiveComponent*> Components;
	GetComponents(Components);
	for (auto Iter = Components.CreateConstIterator(); Iter; ++Iter)
	{
		UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(*Iter);
		if (Component)
		{
			if (Component->IsSimulatingPhysics())
			{
				Mass += Component->GetMass();
			}
		}
	}

	return Mass;
}

void AHovercraft::SetIsFalling(bool IsFalling, int32 ThrustID)
{
	IsFallingArray[ThrustID - 1] = IsFalling;
}

bool AHovercraft::GetIsFalling()
{
	bool Falling = true;
	for (bool Value : IsFallingArray)
	{
		if (!Value)
		{
			Falling = false;
		}
	}

	return Falling;
}

void AHovercraft::ResetHovercraft(USceneComponent* AzimuthGimbal)
{
	if (!StaticMesh || !AzimuthGimbal) { return; }
	if (bIsUpsideDown)
	{
		// Reset Hovercraft

		// get AzimuthGimbal location
		FVector AzimuthGimbalLocation = AzimuthGimbal->GetComponentLocation();
		AzimuthGimbalLocation.Z += ResetHeightModificator;

		// get current AzimuthGimbal rotation
		FRotator AzimuthGimbalRotator = AzimuthGimbal->GetComponentRotation();

		// set roll values to 0
		AzimuthGimbalRotator.Roll = 0.f;

		// transform hovercraft

		// translation
		FVector ResetLocation = StaticMesh->GetComponentLocation();
		ResetLocation.Z += ResetHeightModificator;

		// rotation
		FRotator ResetRotation = GetActorRotation();
		ResetRotation.Roll = 0.f;
		ResetRotation.Pitch = 0.f;

		// apply values
		SetActorLocation(ResetLocation);
		SetActorRotation(ResetRotation);


		// set old AzimuthGimbal and SpringArm rotations
		AzimuthGimbal->SetWorldLocation(AzimuthGimbalLocation);
		AzimuthGimbal->SetWorldRotation(AzimuthGimbalRotator);

	}
}

void AHovercraft::ToggleShouldHover()
{
	bShouldHover = !bShouldHover;

	for (const auto& Thruster : HoverThrusters)
	{
		Thruster->SetShouldHover(bShouldHover);
	}
}

void AHovercraft::ResetHoverHeight()
{
	for (const auto& Thruster : HoverThrusters)
	{
		Thruster->ResetHoverValues();
	}
}

void AHovercraft::ChangeHoverHeightBySteps(int32 Steps)
{
	for (const auto& Thruster : HoverThrusters)
	{
		Thruster->ChangeHoverHeightBySteps(Steps);
	}
}

void AHovercraft::ToggleDrawDebugTraces()
{
	bDrawDebugTraces = !bDrawDebugTraces;
	const FName DebugTraceToDraw = bDrawDebugTraces ? FName("HoverTrace") : NAME_None;
	GetWorld()->DebugDrawTraceTag = DebugTraceToDraw;
}

// Called every frame
void AHovercraft::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!StaticMesh) { return; }

	if (GetIsFalling())
	{
		StaticMesh->AddForce(FVector(0, 0, -1) * DownwardForce);
	}

	// check if hovercraft is upside down
	CheckIsUpsideDown();

}

// Called to bind functionality to input
void AHovercraft::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// setup input bindings in blueprint for soft references

}

