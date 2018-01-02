// Fill out your copyright notice in the Description page of Project Settings.

#include "BMPlayer.h"
#include "components/InputComponent.h"
#include "components/SphereComponent.h"
#include "components/StaticMeshComponent.h"
#include "playfield.h"
// Sets default values
//#pragma optimize("",on)
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

	currentMove=FVector::ZeroVector;
//	bTriggBomb=false;
	bAlive=true;
	PlayField=nullptr;
	spawnedBombs=0;
	remoteTimer=0;

}

// Called when the game starts or when spawned
void ABMPlayer::BeginPlay()
{
	Super::BeginPlay();
	Rotation=GetActorRotation();
	moveStart=moveEnd=GetActorLocation();
}

// Called to bind functionality to input
void ABMPlayer::SetupPlayerInputComponent(UInputComponent* ic)
{
	Super::SetupPlayerInputComponent(ic);
	// input-binding/broadcasts to human player is done in APlayfield
	// AI is setup through a blueprint template
}
void ABMPlayer::AddAvailableBomb()
{
	if(spawnedBombs>0)spawnedBombs--;
	PlayField->SubBomb();
}

// trigger release of a bomb
void ABMPlayer::Fire()
{
	bool remote=false;
	if(remoteTimer!=0)
		remote=true;
	// check if we are in remote state
	if(remote)
	{
		if(spawnedBombs==1)
		{
			// trigger bomb placed
			OnRemoteTrigg.Broadcast();
			return;
		}
	}
	else
	{
		// check if we have reach our bomb limits
		if(spawnedBombs==Bombs)
			return;
	}

	if(BombTemplate)
	{
		// spawn at actor location
		FVector pos=GetActorLocation();
		float distanceLeft=FMath::Abs((currentMove.X+currentMove.Y)/tileSize);
		// check if we are in a move.
		if(distanceLeft!=0)
		{
			if(distanceLeft<0.5f) // move has less than half left.. spawn bomb at destination of move
			{
				// spawn at end point
				pos=moveEnd;
			}
			else				 // move has more than half left.. spawn bomb at start of move
			{
				// spawn at start point
				pos=moveStart;
			}
		}
		PlayField->SpawnTemplate(BombTemplate,pos,this);
		spawnedBombs++;
		PlayField->AddBomb();

	}
}

// initialize move to up or down
void ABMPlayer::Up(float amount)
{
	float a=FMath::Abs(amount);
	// threshold
	if(a < 0.1f)
		return;
	// check if we are in a move
	float acc=currentMove.X+currentMove.Y;
	if(acc!=0)
		return;
	FVector dir(0,0,0);
	FRotator oldRot=Rotation;
	if(amount>0)
	{
		currentMove.X=100;
		Rotation.Yaw=0;
		dir.X=tileSize;
	}
	else
	{
		currentMove.X=-100;
		Rotation.Yaw=180;
		dir.X=-tileSize;
	}
	AddActorLocalRotation(Rotation-oldRot);
	FVector pos=GetActorLocation();
	moveStart=pos;
	moveEnd=pos+dir;
	if(PlayField)
	{
		if(PlayField->CheckUnbreakable(moveEnd))
		{
			currentMove.X=0;
		}
		if(currentMove.X!=0)
		{

			if(PlayField->CheckBreakable(moveEnd)>=0)
			{
				currentMove.X=0;
			}
		}
	}
	if(currentMove.X==0)
	{
		OnMoveEnd.Broadcast();
	}
	else
		OnMoveStart.Broadcast();
}
// initialize move to left or right
void ABMPlayer::Right(float amount)
{
	float a=FMath::Abs(amount);
	if(a < 0.1f)
		return;
	float acc=currentMove.X+currentMove.Y;
	if(acc!=0)
		return;
	FVector dir(0,0,0);
	FRotator oldRot=Rotation;
	if(amount>0)
	{
		currentMove.Y=100;
		Rotation.Yaw=90;
		dir.Y=tileSize;
	}
	else
	{
		currentMove.Y=-100;
		Rotation.Yaw=-90;
		dir.Y=-tileSize;
	}
	AddActorLocalRotation(Rotation-oldRot);
	FVector pos=GetActorLocation();
	moveStart=pos;
	moveEnd=pos+dir;
	if(PlayField)
	{
		if(PlayField->CheckUnbreakable(moveEnd))
		{
			currentMove.Y=0;
		}
		if(currentMove.Y!=0)
		{
			if(PlayField->CheckBreakable(moveEnd)>=0)
			{
				currentMove.Y=0;
			}
		}
	}
	if(currentMove.Y==0)
	{
		OnMoveEnd.Broadcast();
	}
	else
		OnMoveStart.Broadcast();
}
// move pawn amount towards current direction
void ABMPlayer::Move(float step)
{
	FVector stepDir(0,0,0);
	// check and update movement in up/down directions
	if(currentMove.X!=0)
	{
		if(currentMove.X>step)
		{
			currentMove.X-=step;
			stepDir=FVector(step,0,0);
		}
		else if(currentMove.X<-step)
		{
			currentMove.X+=step;
			stepDir=FVector(-step,0,0);
		}
		else
		{
			stepDir=FVector(currentMove.X,0,0);
			currentMove.X=0;
			moveStart=moveEnd;
			OnMoveEnd.Broadcast();
		}

	}
	// else left/right directions
	else if(currentMove.Y!=0)
	{
		if(currentMove.Y>step)
		{
			currentMove.Y-=step;
			stepDir=FVector(0,step,0);	
		}
		else if(currentMove.Y<-step)
		{
			currentMove.Y+=step;
			stepDir=FVector(0,-step,0);	
		}
		else
		{
			stepDir=FVector(0,currentMove.Y,0);	
			currentMove.Y=0;
			moveStart=moveEnd;
			OnMoveEnd.Broadcast();
		}
	}
	AddActorWorldOffset(stepDir,true);
}


class APlayfield* ABMPlayer::GetPlayfield()
{
	return PlayField;
}
void ABMPlayer::Disable()
{
	PlayField->DisablePlayer(this);
	bAlive=false;
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
	return bAlive;
}

bool ABMPlayer::AnythingInfront()
{
	FRotator r(0,Rotation.Yaw,0);
	FVector dir=r.RotateVector(FVector(100,0,0));
	return PlayField->AnyWall(GetActorLocation()+dir);
}
// steps towards dir
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
	// find best direction to move away from an actor using the directions array.
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


// Called every frame
void ABMPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// remote trigger bomb ?
	if(remoteTimer>0)
		remoteTimer-=DeltaTime;
	if(remoteTimer<0)
		remoteTimer=0;

	// step amount this frame
	float stepDistance=Speed*SpeedFactor*DeltaTime;
	Move(stepDistance);

}

//#pragma optimize("",on)
