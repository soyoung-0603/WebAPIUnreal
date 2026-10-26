// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "http.h"
#include "RankingSubsystem.generated.h"

//서버의 GameResult 모델과 매칭되는 언리얼에서 사용할 구조체
USTRUCT(BlueprintType)
struct FGameResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
	int32 Id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
	int32 UserId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
	FString UserName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
	FString UserEmail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
	int32 Score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
	FString Date;
};

//Add / Update / Deleate 결과용 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionSuccessSignature, bool, bSuccess, FString, Verb);

//전체 조회 결과용 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGetRankingSignature, const TArray<FGameResult>&, RankingList);

//단일 조회 결과용 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetSingleRankingSignature, bool, bSuccess, const FGameResult&, SingleResult);

/**
 * 
 */
UCLASS()
class URankingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	//POST = api/ranking
	UFUNCTION(BlueprintCallable, Category = "Ranking")
	void AddGameResult(FGameResult NewResult);

	//GET = api/ranking
	UFUNCTION(BlueprintCallable, Category = "Ranking")
	void GetGameResults();

	//GET = api/ranking/{id}
	UFUNCTION(BlueprintCallable, Category = "Ranking")
	void GetGameResult(int32 TargetId);

	//PUT = api/ranking
	UFUNCTION(BlueprintCallable, Category = "Ranking")
	void UpdateGameResult(FGameResult TargetResult);

	//DELETE = api/ranking/{id}
	UFUNCTION(BlueprintCallable, Category = "Ranking")
	void DeleteGameResult(int32 TargetId);

public:
	//Add / Update / Deleate 완료 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Ranking/Event")
	FOnActionSuccessSignature OnActionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Ranking/Event")
	FOnGetRankingSignature OnGetRankingCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Ranking/Event")
	FOnGetSingleRankingSignature OnGetSingleRankingCompleted;

private:
	//ASP.NET Web 서버 주소
	FString BaseUrl = TEXT("http://localhost:5057/api/Ranking");

private:
	//POST 응답 처리
	void OnAddGameResultResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	
	//GET 전체 응답 처리
	void OnGetGameResultsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	//GET 단일 응답 처리
	void OnGetGameResultResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	//PUT 응답 처리
	void OnUpdateGameResultResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	//DELETE 응답 처리
	void OnDeleteGameResultResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
};
