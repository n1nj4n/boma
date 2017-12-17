// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BMPickup.generated.h"

UCLASS(showCategories=(Collision,Transform,StaticMesh,Material))
class BOMA_API ABMPickup : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta = (AllowPrivateAccess = "true"))
	class USphereComponent*	Collision;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent*	PickupMesh;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UMaterialInterface*	PickupMaterial;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float	createTime;

public:	
	// Sets default values for this actor's properties
	ABMPickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	
	
};
