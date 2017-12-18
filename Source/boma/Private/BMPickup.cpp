// Fill out your copyright notice in the Description page of Project Settings.

#include "BMPickup.h"
#include "components/SphereComponent.h"
#include "components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABMPickup::ABMPickup()
{
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->SetSphereRadius(49.9f,false);
	Collision->SetCollisionProfileName("BMPickup");

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetCollisionProfileName("NoCollision");
	PickupMesh->SetupAttachment(Collision);
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	createTime=0;
	RotationSpeed=90.f;

}

// Called when the game starts or when spawned
void ABMPickup::BeginPlay()
{
	Super::BeginPlay();
	if(PickupMaterial)
		PickupMesh->SetMaterial(0,PickupMaterial);
	createTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	
	
}

void ABMPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float rad=FMath::DegreesToRadians(RotationSpeed*DeltaTime);
	PickupMesh->AddLocalRotation(FQuat(FVector(0,0,1),rad));

}

