// Copyright Yoan Rock

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "NBodyManager.generated.h"

USTRUCT()
struct FBodyEntity {
	GENERATED_BODY()

	FBodyEntity() = default;

	UPROPERTY()
	FVector2D Position;

	UPROPERTY()
	FVector2D Velocity;

	UPROPERTY()
	float Mass = 1.0f;

	UPROPERTY()
	int32 Index;
};

UCLASS()
class NBODYSIMULATION_API ANBodyManager : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	ANBodyManager();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Instanced, NoClear)
	UInstancedStaticMeshComponent* InstancedMesh;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation Parameters")
	bool IsGravityEnabled = true;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation Parameters")
	int BodyNum = 2500;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation Parameters")
	float PlacementRadius = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation Parameters")
	float BaseInitialVelocity = 500.0f;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation Parameters")
	float BodyDisplayScale = 0.02f;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation Parameters")
	float G = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation Parameters")
	float MinMass = 20.0f;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation Parameters")
	float MaxMass = 100.0f;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation Parameters")
	float MinimumGravityDistance = 100.0f;  // prevents division by zero and forces too high

	/* I didn't have enough time to make a cleaner solution than having this 2 tweakables variables */
	UPROPERTY(EditAnywhere, Category = "NBody Simulation Parameters")
	float ScreenWidth = 15.0f;

	UPROPERTY(EditAnywhere, Category = "NBody Simulation Parameters")
	float ScreenHeight = 11.f;

private:
	void InitBodies();
	void InitScreenEdges();

	void GravityStep(float DeltaTime);
	void UpdatePositionStep(float DeltaTime);

	void TeleportIfOutsideScreen(FBodyEntity& Body);

	static FVector TranslationFrom2DCoordinates(const FVector2D& XYCoordinates);

	UPROPERTY()
	TArray<FBodyEntity> Bodies;

	UPROPERTY()
	TArray<FTransform> Transforms;

	float ScreenEdgeX = 0.f;
	float ScreenEdgeY = 0.f;
};
