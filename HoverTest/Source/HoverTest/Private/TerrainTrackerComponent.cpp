// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainTrackerComponent.h"
#include "Engine/Classes/Kismet/GameplayStatics.h"
#include "TerrainManager.h"


// Sets default values for this component's properties
UTerrainTrackerComponent::UTerrainTrackerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UTerrainTrackerComponent::OnUnregister()
{
	if (TerrainManager)
	{
		TerrainManager->RemoveTrackedActor(GetOwner());
		CurrentSector = TerrainManager->CalculateSectorFromLocation(GetOwner()->GetActorLocation());
	}
	Super::OnUnregister();
}

// Called when the game starts
void UTerrainTrackerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	TerrainManager = GetWorldTerrainManager();
	if (TerrainManager)
	{
		// register parent actor at terrain manager so it gets tracked
		TerrainManager->AddActorToTrack(GetOwner());
		CurrentSector = TerrainManager->CalculateSectorFromLocation(GetOwner()->GetActorLocation());

	}



}


// Called every frame
void UTerrainTrackerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	// on each tick, check if the parent actor changed sector
	if (TerrainManager)
	{
		FIntVector2D SectorThisTick = TerrainManager->CalculateSectorFromLocation(GetOwner()->GetActorLocation());
		//UE_LOG(LogTemp, Warning, TEXT("Sector this tick: %s"), *SectorThisTick.ToString());
		//UE_LOG(LogTemp, Error, TEXT("Current sector: %s"), *CurrentSector.ToString());
		if (SectorThisTick != CurrentSector)
		{
			//UE_LOG(LogTemp, Warning, TEXT("%s has moved sectors"), *GetOwner()->GetName());
			TerrainManager->HandleTrackedActorChangedSector(GetOwner(), CurrentSector, SectorThisTick);
			CurrentSector = SectorThisTick;
		}
	}
}

ATerrainManager * UTerrainTrackerComponent::GetWorldTerrainManager()
{
	TArray<AActor*> FoundManagers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATerrainManager::StaticClass(), FoundManagers);
	// if there is more than one TerrainManager (or none at all) we can't operate
	if (FoundManagers.Num() != 1)
	{
		UE_LOG(LogTemp, Error, TEXT("Make sure there is exactly one TerrainManager in the level!"));
		return nullptr;
	}
	ATerrainManager* Manager = Cast<ATerrainManager>(FoundManagers[0]);
	return Manager;

}

