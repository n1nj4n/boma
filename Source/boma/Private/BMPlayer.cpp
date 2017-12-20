// Fill out your copyright notice in the Description page of Project Settings.

#include "BMPlayer.h"
#include "components/InputComponent.h"
#include "components/SphereComponent.h"
#include "components/StaticMeshComponent.h"
#include "playfield.h"
// Sets default values
#pragma optimize("",off)
ABMPlayer::ABMPlayer()
{

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Collision->SetSphereRadius(49.9f,false);

	SetRootComponent(Collision);
	Mesh->SetupAttachment(Collision);
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Bombs=1;
	BombStrength=1;
	SpeedFactor=1;
	Speed=100.f;

	up=0;
	right=0;
	bTriggBomb=false;
	alive=true;
	spawnedBombs=0;
	remoteTimer=0;

}

// Called when the game starts or when spawned
void ABMPlayer::BeginPlay()
{
	Super::BeginPlay();
	Rotation=GetActorRotation();
	moveTo=moveFrom=GetActorLocation();
}

// Called to bind functionality to input
void ABMPlayer::SetupPlayerInputComponent(UInputComponent* ic)
{
	Super::SetupPlayerInputComponent(ic);
//	ic->BindAction("Human1Fire",IE_Released,this,&ABMPlayer::Fire);
//	ic->BindAxis("Human1Up",this,&ABMPlayer::Up);
//	ic->BindAxis("Human1Right",this,&ABMPlayer::Right);

}
void ABMPlayer::AddAvailableBomb()
{
	if(spawnedBombs>0)spawnedBombs--;
	MainPawn->SubBomb();

}
void ABMPlayer::Fire()
{
	bool remote=false;
	if(remoteTimer!=0)
		remote=true;
	if(!remote)
	{
		if(spawnedBombs==Bombs)
			return;
	}
	else
	{
		if(spawnedBombs==1)
		{
			OnRemoteTrigg.Broadcast();
			return;
		}
	}


	if(BombTemplate)
	{
		FVector pos=GetActorLocation();
		if(!(up==0 && right==0))
		{
			if(up<50.f && right<50.f)
			{
				pos=moveTo;
			}
			else if(up>50.f || right>50.f)
			{
				pos=moveFrom;
			}
		}
		MainPawn->SpawnTemplate(BombTemplate,pos,this);
		spawnedBombs++;
		MainPawn->AddBomb();

	}
}
void ABMPlayer::Up(float amount)
{
	float a=FMath::Abs(amount);
	if(a < 0.1f)
		return;
	if(right!=0 || up!=0)
		return;
	FVector dir(0,0,0);
	FRotator oldRot=Rotation;
	if(amount>0){up=100;Rotation.Yaw=0;dir.X=tileSize;}
	else {up=-100;Rotation.Yaw=180;dir.X=-tileSize;}
	AddActorLocalRotation(Rotation-oldRot);
	FVector pos=GetActorLocation();
	moveFrom=pos;
	moveTo=pos+dir;
	if(MainPawn)
	{
		if(MainPawn->CheckUnbreakable(moveTo))
		{
			up=0;
		}
		if(up!=0)
		{

			if(MainPawn->CheckBreakable(moveTo)>=0)
			{
				up=0;
			}
		}
	}
	if(up==0)
		OnLastMoveDone.Broadcast();
}
void ABMPlayer::Right(float amount)
{
	float a=FMath::Abs(amount);
	if(a < 0.1f)
		return;
	if(right!=0 || up!=0)
	{
		return;
	}
	FVector dir(0,0,0);
	FRotator oldRot=Rotation;
	if(amount>0){right=100;Rotation.Yaw=90;dir.Y=tileSize;}
	else {right=-100;Rotation.Yaw=-90;dir.Y=-tileSize;}
	AddActorLocalRotation(Rotation-oldRot);
	FVector pos=GetActorLocation();
	moveFrom=pos;
	moveTo=pos+dir;
	if(MainPawn)
	{
		if(MainPawn->CheckUnbreakable(moveTo))
		{
			right=0;
		}
		if(right!=0)
		{
			if(MainPawn->CheckBreakable(moveTo)>=0)
			{
				right=0;
			}
		}
	}
	if(right==0)
		OnLastMoveDone.Broadcast();
}
void ABMPlayer::Move(float step)
{
	FVector stepDir(0,0,0);


	if(up!=0)
	{
		if(up>step)
		{
			up-=step;
			stepDir=FVector(step,0,0);
		}
		else if(up<-step)
		{
			up+=step;
			stepDir=FVector(-step,0,0);
		}
		else
		{
			stepDir=FVector(up,0,0);
			up=0;
			OnLastMoveDone.Broadcast();
		}

	}
	else if(right!=0)
	{
		if(right>step)
		{
			right-=step;
			stepDir=FVector(0,step,0);	
		}
		else if(right<-step)
		{
			right+=step;
			stepDir=FVector(0,-step,0);	
		}
		else
		{
			stepDir=FVector(0,right,0);	
			right=0;
			OnLastMoveDone.Broadcast();
		}

	}
	AddActorWorldOffset(stepDir,true);
}

// Called every frame
void ABMPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(remoteTimer>0)
		remoteTimer-=DeltaTime;
	if(remoteTimer<0)
		remoteTimer=0;
	Move(Speed*SpeedFactor*DeltaTime);
	if(bTriggBomb)
	{
		if(right==0 && up==0)
		{
			bTriggBomb=false;
			Fire();
		}
	}
}
class APlayfield* ABMPlayer::GetPlayfield()
{
	return MainPawn;
}
void ABMPlayer::KillYourself()
{
	MainPawn->RemovePlayer(this);
	alive=false;
}

void ABMPlayer::EnableRemote(float duration)
{
	if(duration<0)
		return;
	remoteTimer+=duration;
}

float ABMPlayer::GetRemoteTimer()
{
	return remoteTimer;
}
bool ABMPlayer::IsAlive()
{
	return alive;
}

bool ABMPlayer::AnyDestructablesAround()
{
	int up=MainPawn->CheckBreakable(GetActorLocation()		+FVector(100,0,0));
	if(up>=0)
		return true;
	int down=MainPawn->CheckBreakable(GetActorLocation()	+FVector(-100,0,0));
	if(down>=0)
		return true;
	int left=MainPawn->CheckBreakable(GetActorLocation()	+FVector(0,-100,0));
	if(left>=0)
		return true;
	int right=MainPawn->CheckBreakable(GetActorLocation()	+FVector(0,100,0));
	if(right>=0)
		return true;
	return false;
}
bool ABMPlayer::AnythingInfront()
{
	FRotator r(0,Rotation.Yaw,0);
	FVector dir=r.RotateVector(FVector(100,0,0));
	return MainPawn->AnyWall(GetActorLocation()+dir);
}

TArray<FVector> ABMPlayer::ValidStepDirections()
{
	FRotator r(0,Rotation.Yaw,0);
	FVector frontdir=r.RotateVector(FVector(100,0,0));
	FVector rightdir(-frontdir.Y,frontdir.X,0);
	FVector pos=GetActorLocation();
	TArray<FVector> ret;
	FVector directions[4]={frontdir,-rightdir,rightdir,-frontdir};
	for(int i=0;i<4;i++)
	{
		if(!MainPawn->AnyWall(pos+directions[i]))
		{
			ret.Add(directions[i]);
		}
	}
	return ret;
}
void ABMPlayer::Step(FVector dir)
{
	if(dir.SizeSquared()==0)
	{
		if(Rotation.Yaw==0)Up(1);
		else if(Rotation.Yaw==-90)Right(-1);
		else if(Rotation.Yaw== 90)Right(1);
		else Up(-1);
		return;
	}
	if((dir|FVector(1,0,0)) >0.99f)
		Up(1);
	else if((dir|FVector(0,-1,0))>0.99f)
		Right(-1);
	else if((dir|FVector(-1,0,0))>0.99f)
		Up(-1);
	else if((dir|FVector(0,1,0))>0.99f)
		Right(1);
}

AActor* ABMPlayer::ClosestOfClass(UClass* type, float radius)
{
	ULevel* pLev=GWorld->GetLevel(0);
	AActor* closest=nullptr;
	float dS=radius*radius;
	for(int i=0;i<pLev->Actors.Num();i++)
	{
		AActor* a=pLev->Actors[i];
		if(!a)
			continue;
		UClass* ac=a->GetClass();
		if(ac==type)
		{
			FVector ad=a->GetActorLocation()-GetActorLocation();
			float distSq=ad.SizeSquared();
			if(dS>distSq)
			{
				dS=distSq;
				closest=a;
			}
		}
	}
	return closest;
}
float ABMPlayer::DistanceOfClosestOfClass(UClass* type, float radius)
{
	ULevel* pLev=GWorld->GetLevel(0);
	AActor* closest=nullptr;
	float dS=radius*radius;
	for(int i=0;i<pLev->Actors.Num();i++)
	{
		AActor* a=pLev->Actors[i];
		if(a->StaticClass()==type)
		{
			FVector ad=a->GetActorLocation()-GetActorLocation();
			float distSq=ad.SizeSquared();
			if(dS>distSq)
			{
				dS=distSq;
				closest=a;
			}
		}
	}
	return FMath::Sqrt(dS);
}
FVector ABMPlayer::MoveAwayFrom(AActor* object, const TArray<FVector>& directions)
{
	FVector ret(0,0,0);
	if(!object)
		return directions.Num()?directions[0]:FVector(0,0,0);
	FVector from=object->GetActorLocation();
	FVector me=GetActorLocation();
	float moveToDist=(GetActorLocation()-from).SizeSquared();
	int idx=0;
	for(int i=0;i<directions.Num();i++)
	{
		float di=((me+directions[i])-from).SizeSquared();
		if(di>moveToDist)
		{
			moveToDist=di;
			ret=directions[i];
		}
	}
	return ret;
}

#pragma optimize("",on)
