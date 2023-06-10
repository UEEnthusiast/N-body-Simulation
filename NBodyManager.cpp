// Copyright Yoan Rock

#include "NBodyManager.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"

const float MAX_TICK = 0.0167; // to have stable simulation steps

ANBodyManager::ANBodyManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = TG_DuringPhysics;
	InstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
	SetRootComponent(InstancedMesh);
}

void ANBodyManager::InitBodies()
{
	Bodies.SetNumUninitialized(BodyNum);
	Transforms.SetNumUninitialized(BodyNum);
	for (int32 index = 0; index < BodyNum; ++index) {

		FVector2D randomPosition(FMath::RandPointInCircle(PlacementRadius));
		float radialSpeedFactor = PlacementRadius / randomPosition.Size();
		FVector2D randomVelocity{ FMath::FRandRange(BaseInitialVelocity - 100.0f, BaseInitialVelocity + 100.0f) / radialSpeedFactor, 0 };
		randomVelocity = randomVelocity.GetRotated(90.0f + FMath::RadiansToDegrees(FMath::Atan2(randomPosition.Y, randomPosition.X)));
		float mass = FMath::FRandRange(MinMass, MaxMass);
		float meshScale = FMath::Sqrt(mass) * BodyDisplayScale;
		
		Transforms[index] = { FRotator(), TranslationFrom2DCoordinates(randomPosition),	FVector(meshScale, meshScale, 1.f) };

		Bodies[index] = FBodyEntity{ randomPosition, randomVelocity, mass, index };
	}
	InstancedMesh->AddInstances(Transforms, false);
}

void ANBodyManager::InitScreenEdges()
{
	APawn* myPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!ensure(myPawn != nullptr))
	{
		return;
	}

	UCameraComponent* cameraComponent = myPawn->FindComponentByClass<UCameraComponent>();
	if (!ensure(cameraComponent != nullptr))
	{
		return;
	}

	FMinimalViewInfo viewInfo;
	cameraComponent->GetCameraView(0.0f, viewInfo);

	float viewWidth = viewInfo.FOV * viewInfo.AspectRatio;
	float viewHeight = viewInfo.FOV;

	ScreenEdgeX = viewInfo.Location.X + viewWidth * ScreenWidth;
	ScreenEdgeY = viewInfo.Location.Y + viewHeight * ScreenHeight;
}

void ANBodyManager::BeginPlay()
{
	Super::BeginPlay();
	InitBodies();
	InitScreenEdges();
}

void ANBodyManager::GravityStep(float DeltaTime)
{
	ParallelFor(Bodies.Num(), [&](int32 index) {
		FVector2D acceleration(0.0f, 0.0f);
		for (const FBodyEntity& AffectingBody : Bodies) {
			if (AffectingBody.Index == Bodies[index].Index) continue; // exclude self
			float distance = FVector2D::Distance(Bodies[index].Position, AffectingBody.Position);
			distance = FMath::Max(distance, MinimumGravityDistance); // avoids division by zero
			acceleration += AffectingBody.Mass / distance * G / distance * (AffectingBody.Position - Bodies[index].Position) / distance;
		}
		Bodies[index].Velocity += acceleration * DeltaTime;
		});
}

void ANBodyManager::UpdatePositionStep(float DeltaTime)
{
	for (FBodyEntity& Body : Bodies) {
		Body.Position += Body.Velocity * DeltaTime;
		TeleportIfOutsideScreen(Body);
		Transforms[Body.Index].SetTranslation(TranslationFrom2DCoordinates(Body.Position));
	}
	InstancedMesh->BatchUpdateInstancesTransforms(0, Transforms, false, true);
}

void ANBodyManager::TeleportIfOutsideScreen(FBodyEntity& Body)
{
	if (Body.Position.X > ScreenEdgeX)
	{
		Body.Position.X = -ScreenEdgeX;
	}
	else if (Body.Position.X < -ScreenEdgeX)
	{
		Body.Position.X = ScreenEdgeX;
	}
	if (Body.Position.Y > ScreenEdgeY)
	{
		Body.Position.Y = -ScreenEdgeY;
	}
	else if (Body.Position.Y < -ScreenEdgeY)
	{
		Body.Position.Y = ScreenEdgeY;
	}
}

void ANBodyManager::Tick(float DeltaTime)
{
	if (DeltaTime > MAX_TICK) {
		DeltaTime = MAX_TICK;
	}
	Super::Tick(DeltaTime);
	if (IsGravityEnabled) 
	{
		GravityStep(DeltaTime);
	}
	UpdatePositionStep(DeltaTime);
}

FVector ANBodyManager::TranslationFrom2DCoordinates(const FVector2D& XYCoordinates) 
{
	return FVector(XYCoordinates.Y, XYCoordinates.X, 0.0f);
}
