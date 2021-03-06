// Fill out your copyright notice in the Description page of Project Settings.

#include "Hovercraft.h"
#include "Classes/Components/InputComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Components/PrimitiveComponent.h"
#include "Classes/Engine/World.h"
#include "HoverThruster.h"
#include "HovercraftPlayerController.h"
#include "Classes/Components/SceneComponent.h"
#include "MomentumThruster.h"
#include "Classes/Curves/CurveFloat.h"
#include "Public/TimerManager.h"
#include "Classes/Materials/MaterialInstanceDynamic.h"
#include "Camera/CameraComponent.h"
#include "Classes/Components/PostProcessComponent.h"
#include "TerrainTrackerComponent.h"
#include "HoverTestGameModeProceduralLevel.h"
#include "TerrainManager.h"
#include "Classes/GameFramework/SpringArmComponent.h"


// Sets default values
AHovercraft::AHovercraft()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TerrainTrackerComponent = CreateDefaultSubobject<UTerrainTrackerComponent>(TEXT("TerrainTracker"));
}

// Called when the game starts or when spawned
void AHovercraft::BeginPlay()
{
	Super::BeginPlay();

	// needs to be called before IsFallingArray.Init()
	InitializeThrusters();

	IsFallingArray.Init(false, NumberOfThrusters);	

	// check if pawn is player controlled
	AHovercraftPlayerController* PC = Cast<AHovercraftPlayerController>(GetController());
	if (PC)
	{
		bIsPlayerControlled = true;
	}
	else
	{
		bIsPlayerControlled = false;
	}

}

void AHovercraft::MoveForward(float Value, EControllerType InputControllerType)
{
	if (Value != 0)
	{
		ControllerType = InputControllerType;
	}
	if (!StaticMesh) { return; }
	if (bIsResetting) { return; }
	Value = FMath::Clamp<float>(Value, -1.f, 1.f);
	FVector ForceDirection = GetActorForwardVector();
	if (GetIsFalling())
	{
		ForceDirection.Z = 0.f;
		ForceDirection = ForceDirection.GetSafeNormal();
	}

	FVector ForceToApply = ForceDirection * ForwardAcceleration * Value;
	if (GetIsFalling())
	{
		ForceToApply *= ForwardAirborneControl;
	}

	StaticMesh->AddForce(ForceToApply);
}

void AHovercraft::MoveRight(float Value, EControllerType InputControllerType)
{
	if (Value != 0)
	{
		ControllerType = InputControllerType;
	}
	if (!StaticMesh) { return; }
	if (bIsResetting) { return; }
	Value = FMath::Clamp<float>(Value, -1.f, 1.f);
	FVector ForceDirection = GetActorRightVector();
	if (GetIsFalling())
	{
		ForceDirection.Z = 0;
		ForceDirection = ForceDirection.GetSafeNormal();
	}

	FVector ForceToApply = ForceDirection * SidewardAcceleration * Value;
	if (GetIsFalling())
	{
		ForceToApply *= SidewardsAirborneControl;
	}

	StaticMesh->AddForce(ForceToApply);
}

void AHovercraft::RotateRight(float Value, EControllerType InputControllerType)
{
	if (Value != 0)
	{
		ControllerType = InputControllerType;
	}
	if (!StaticMesh || !GetWorld()) { return; }
	if (bIsResetting) { return; }
	/*Value = FMath::Clamp<float>(Value, -1.f, 1.f);
	FRotator Rotator = FRotator(0, RotationSpeed * GetWorld()->DeltaTimeSeconds * Value, 0);
	StaticMesh->AddWorldRotation(Rotator);*/

	Value = FMath::Clamp<float>(Value, -1.f, 1.f);
	// momentum force
	if (!RightRotationPoint || !LeftRotationPoint) { return; }
	FVector ForceToApply = StaticMesh->GetRightVector() * Value * RotationForce;
	if (Value > 0)
	{
		StaticMesh->AddForceAtLocation(ForceToApply, LeftRotationPoint->GetComponentLocation());
	}
	else
	{
		StaticMesh->AddForceAtLocation(ForceToApply, RightRotationPoint->GetComponentLocation());
	}
}


void AHovercraft::SetStaticMeshReference(UStaticMeshComponent * MeshToSet)
{
	StaticMesh = MeshToSet;

	if (!StaticMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid static mesh assigned to %s"), *GetName());
		return;
	}

	if (StaticMesh->GetNumMaterials() < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("Static mesh on %s has not assigned 2 materials!"), *GetName());
	}

	DynamicMaterialStandard = UMaterialInstanceDynamic::Create(StaticMesh->GetMaterial(0), this);
	if (!DynamicMaterialStandard)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create standard dynamic material instance on %s"), *GetName());
	}
	DynamicMaterialTranslucent = UMaterialInstanceDynamic::Create(StaticMesh->GetMaterial(1), this);
	if (!DynamicMaterialTranslucent)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create translucent dynamic material instance on %s. Does your static mesh model has 2 materials assigned?"), *GetName());
	}

	if (DynamicMaterialStandard)
	{
		if (DynamicMaterialStandard->GetBlendMode() != EBlendMode::BLEND_Opaque)
		{
			UE_LOG(LogTemp, Error, TEXT("Material at index 0 in %s on %s needs blend mode set to opaque!"), *StaticMesh->GetName(), *GetName());
		}
	}

	if (DynamicMaterialTranslucent)
	{
		if (DynamicMaterialTranslucent->GetBlendMode() != EBlendMode::BLEND_Translucent)
		{
			UE_LOG(LogTemp, Error, TEXT("Material at index 1 in %s on %s needs blend mode set to translucent!"), *StaticMesh->GetName(), *GetName());
		}
	}

	if (StaticMesh && DynamicMaterialStandard)
	{
		StaticMesh->SetMaterial(0, DynamicMaterialStandard);
	}
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

void AHovercraft::ResetHovercraft(USpringArmComponent* SpringArmComponent)
{
	if (!StaticMesh || bIsResetting || !SpringArmComponent) { return; }

	// transform hovercraft

	// translation
	FVector ResetLocation;
	FRotator ResetRotation;
	AHovercraftPlayerController* PC = Cast<AHovercraftPlayerController>(GetController());
	if (PC)
	{
		ResetLocation = PC->GetResetPosition();
		/*if (HoverThrusters[0])
		{
			ResetLocation.Z = HoverThrusters[0]->HoverHeight;
		}*/
		ResetLocation.Z += ResetHeightModificator;
		ResetRotation = FRotator(0.f, PC->GetResetYaw(), 0.f);
	}
	else
	{
		ResetLocation = StaticMesh->GetComponentLocation();
		ResetLocation.Z += ResetHeightModificator;
		ResetRotation = GetActorRotation();
		ResetRotation.Pitch = 0.f;
		ResetRotation.Roll = 0.f;
		UE_LOG(LogTemp, Warning, TEXT("Could not find PlayerController in %s, used local values instead"), *GetName());
	}

	// set velocity to zero
	StaticMesh->SetPhysicsLinearVelocity(FVector(0.f, 0.f, 0.f));
	StaticMesh->SetPhysicsAngularVelocity(FVector(0.f, 0.f, 0.f));

	// check if we are in procedural level (e.g. by checking gamemode castable to procedural game mode)
	UWorld* World = GetWorld();
	if (World)
	{
		AHoverTestGameModeProceduralLevel* GameMode = Cast<AHoverTestGameModeProceduralLevel>(World->GetAuthGameMode());
		if (GameMode)
		{
			// check if location where we want to reset is covered by a tile
			ATerrainManager* TerrainManager = GameMode->GetTerrainManager();
			if (TerrainManager)
			{
				if (TerrainManager->IsLocationCoveredByTile(ResetLocation))
				{
					// apply values
					ApplyResetValues(ResetLocation, ResetRotation);
				}
				else
				{
					// disable camera lag
					SpringArmComponent->bEnableCameraLag = false;
					SAC = SpringArmComponent;
					// switch over to procedural pawn
					GameMode->SwitchToDefaultPawnAndStartMultipointTransition(this, ResetLocation, ResetRotation);
					// notify terrain manager
					TerrainManager->BeginTileGenerationForReset(ResetLocation);

				}
			}
		}
		else
		{
			// apply values
			ApplyResetValues(ResetLocation, ResetRotation);
		}
	}
	else
	{
		// apply values
		ApplyResetValues(ResetLocation, ResetRotation);
	}
}

void AHovercraft::OnMultipointTransitionResetComplete()
{
	if (!SAC) { return; }
	SAC->bEnableCameraLag = true;
}

void AHovercraft::ToggleShouldHover()
{
	if (bIsResetting) { return; }
	bShouldHover = !bShouldHover;

	for (const auto& Thruster : HoverThrusters)
	{
		Thruster->SetShouldHover(bShouldHover);
	}
}

void AHovercraft::ResetHoverHeight()
{
	if (bIsResetting) { return; }
	for (const auto& Thruster : HoverThrusters)
	{
		Thruster->ResetHoverValues();
	}
}

void AHovercraft::ChangeHoverHeightBySteps(int32 Steps)
{
	if (bIsResetting) { return; }
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

bool AHovercraft::GetStaticMeshLocation(FVector& Location, float& Yaw)
{
	if (!StaticMesh) { return false; }
	Location = StaticMesh->GetComponentLocation();
	Yaw = StaticMesh->GetComponentRotation().Yaw;
	return true;
}

int32 AHovercraft::GetSpeed() const
{
	FVector Velocity = GetVelocity();// *StaticMesh->GetForwardVector();
	return int32(Velocity.Size() / 100.f);
}

void AHovercraft::SetMomentumThrusterReferences(UMomentumThruster * RightMomentumReference, UMomentumThruster * LeftMomentumReference, UMomentumThruster * BackMomentumReference)
{
	RightMomentumThruster = RightMomentumReference;
	LeftMomentumThruster = LeftMomentumReference;
	BackMomentumThruster = BackMomentumReference;
}

void AHovercraft::SetRotationPointReferences(USceneComponent * RightRotationPointReference, USceneComponent * LeftRotationPointReference)
{
	RightRotationPoint = RightRotationPointReference;
	LeftRotationPoint = LeftRotationPointReference;
}

int32 AHovercraft::GetIndexOfLastCheckpoint() const
{
	return IndexOfLastCheckpoint;
}

void AHovercraft::SetIndexOfLastCheckpoint(int32 NewCheckpointID)
{
	IndexOfLastCheckpoint = NewCheckpointID;
}

void AHovercraft::SetStopLapTime(bool ShouldStopLapTime)
{
	bShouldStopTime = ShouldStopLapTime;
}

float AHovercraft::ResetLapTimer()
{
	float Buffer = LapTime;
	LapTime = 0.f;
	return Buffer;
}

float AHovercraft::GetLapTime() const
{
	return LapTime;
}

UTerrainTrackerComponent * AHovercraft::GetTerrainTrackerComponent() const
{
	return TerrainTrackerComponent;
}

void AHovercraft::ToggleShowControls()
{
	bShowControls = !bShowControls;
}

bool AHovercraft::GetShowControls() const
{
	return bShowControls;
}

void AHovercraft::SetResetCurveReference(UCurveFloat * CurveReference)
{
	ResetCurve = CurveReference;
}

void AHovercraft::SetPostProcessMaterialReference(UMaterialInterface * MaterialInterface, UCameraComponent * CameraToAdd)
{
	if (!MaterialInterface || !CameraToAdd) { return; }
	CameraComponent = CameraToAdd;
	DynamicMaterialPostProcess = UMaterialInstanceDynamic::Create(MaterialInterface, this);
	DynamicMaterialPostProcess->SetScalarParameterValue("BlurAmount", 0.f);
	DynamicMaterialPostProcess->SetScalarParameterValue("BlurRadius", RadialBlurRadius);
	CameraComponent->PostProcessSettings.AddBlendable(DynamicMaterialPostProcess, 1.f);
	bIsRadialBlurApplied = false;
}

void AHovercraft::OnResetComplete()
{
	//SetActorEnableCollision(true);
	bIsResetting = false;
	ResetCurveTimer = 0.f;
	if (DynamicMaterialStandard && StaticMesh)
	{
		StaticMesh->SetMaterial(0, DynamicMaterialStandard);
	}
}

bool AHovercraft::HandleResetStuff(float DeltaTime)
{
	if (bIsResetting)
	{
		ResetCurveTimer += DeltaTime;
		ResetCurveTimer = (ResetCurveTimer >= ResetCurveLength) ? (ResetCurveTimer - ResetCurveLength) : ResetCurveTimer;

		if (ResetCurve)
		{
			float ResetCurveValue = ResetCurve->GetFloatValue(ResetCurveTimer);
			if (!DynamicMaterialTranslucent)
			{
				UE_LOG(LogTemp, Error, TEXT("Could not set dynamic material in %s."), *GetName());
			}
			else
			{
				if (!StaticMesh)
				{
					UE_LOG(LogTemp, Error, TEXT("Could not assign dynamic material to static mesh in %s because no static mesh is available."), *GetName());
				}
				else
				{
					DynamicMaterialTranslucent->SetScalarParameterValue("Alpha", ResetCurveValue);
					StaticMesh->SetMaterial(0, DynamicMaterialTranslucent);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find a ResetCurve in %s."), *GetName());
		}
	}

	return bIsResetting;
}

void AHovercraft::HandlePostProcessStuff()
{
	if (!DynamicMaterialPostProcess || !CameraComponent) { return; }
	if (!bIsRadialBlurApplied && GetSpeed() >= 70)
	{
		bIsRadialBlurApplied = true;
		CameraComponent->PostProcessSettings.RemoveBlendable(DynamicMaterialPostProcess);
		DynamicMaterialPostProcess->SetScalarParameterValue("BlurRadius", RadialBlurRadius);
		DynamicMaterialPostProcess->SetScalarParameterValue("BlurAmount", RadialBlurMaxStrength);
		CameraComponent->PostProcessSettings.AddBlendable(DynamicMaterialPostProcess, 1.f);
	}
	else if (bIsRadialBlurApplied && GetSpeed() < 70)
	{
		bIsRadialBlurApplied = false;
		CameraComponent->PostProcessSettings.RemoveBlendable(DynamicMaterialPostProcess);
		DynamicMaterialPostProcess->SetScalarParameterValue("BlurAmount", 0.f);
		DynamicMaterialPostProcess->SetScalarParameterValue("BlurRadius", RadialBlurRadius);
		CameraComponent->PostProcessSettings.AddBlendable(DynamicMaterialPostProcess, 1.f);
	}
}

// Called every frame
void AHovercraft::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldStopTime)
	{
		LapTime += DeltaTime;
	}

	HandlePostProcessStuff();

	// do stuff when resetting
	HandleResetStuff(DeltaTime);

	if (!StaticMesh) { return; }

	if (GetIsFalling())
	{
		StaticMesh->AddForce(FVector(0, 0, -1) * DownwardForce);
	}

	// check if hovercraft is upside down
	CheckIsUpsideDown();

}

EControllerType AHovercraft::GetControllerType() const
{
	if (!bIsPlayerControlled) { return EControllerType::ECT_None; }
	else
	{
		return ControllerType;
	}
}

void AHovercraft::ApplyResetValues(const FVector ResetLocation, const FRotator ResetRotation)
{
	SetActorLocation(ResetLocation);
	SetActorRotation(ResetRotation);

	bIsResetting = true;

	//SetActorEnableCollision(false);
	GetWorld()->GetTimerManager().SetTimer(ResetTimerHandle, this, &AHovercraft::OnResetComplete, TimeNeededForReset, false);
}

// Called to bind functionality to input
void AHovercraft::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// setup input bindings in blueprint for soft references

}

void AHovercraft::SetIsPlayerControlled(bool IsPlayerControlled)
{
	bIsPlayerControlled = IsPlayerControlled;
}

void AHovercraft::SetNewProceduralCheckpointID(const uint32 NewID)
{
	CurrentProceduralCheckpointID = NewID;
}

uint32 AHovercraft::GetCurrentProceduralCheckpointID() const
{
	return CurrentProceduralCheckpointID;
}

