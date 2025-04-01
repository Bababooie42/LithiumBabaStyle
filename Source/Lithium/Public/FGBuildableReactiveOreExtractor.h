#pragma once

#include "Buildables/FGBuildableResourceExtractor.h"
#include "FGBuildableReactiveOreExtractor.generated.h"

UCLASS()
class LITHIUM_API AFGBuildableReactiveOreExtractor : public AFGBuildableResourceExtractor
{
    GENERATED_BODY()

public:
    AFGBuildableReactiveOreExtractor();

    // Begin AActor interface
    virtual void BeginPlay() override;
    // End AActor interface

    //~ Begin FGBuildableFactory Interface
    virtual bool CanProduce_Implementation() const override;
    virtual void Factory_Tick(float dt) override;
    virtual void Factory_TickProducing(float dt) override;
    //~ End FGBuildableFactory Interface

    /** Get the input inventory where catalyst items are stored */
    UFUNCTION(BlueprintPure, Category = "Factory")
    class UFGInventoryComponent* GetInputInventory() const { return mInputInventory; }

    /** Get the catalyst consumption rate per minute converted for displaying in the UI*/
    UFUNCTION(BlueprintPure, Category = "Extraction")
    float GetCatalystConsumptionPerMinute() const;

    /** Get the quantity of items extracted per minute converted for displaying in the UI */
    float GetNumExtractedItemsPerCycle() const { return mCatalystItemsPerCycle; }

    /** Get the quantity of items extracted per minute converted for displaying in the UI */
    UFUNCTION(BlueprintPure, Category = "Extraction")
    float GetNumExtractedItemsPerMinute() const;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
    TSubclassOf<class UFGItemDescriptor> mOutputItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
    TSubclassOf<class UFGItemDescriptor> mCatalystItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (ClampMin = "1"))
    int32 mCatalystItemsPerCycle;

private:
    UPROPERTY()
    class UFGInventoryComponent* mInputInventory;

    UPROPERTY()
    class UFGFactoryConnectionComponent* mInputInventoryConnection;

    UPROPERTY()
    class UFGFactoryConnectionComponent* mOutputInventoryConnection;
};