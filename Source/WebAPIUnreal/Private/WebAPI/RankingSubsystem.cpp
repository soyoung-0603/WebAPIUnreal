// Fill out your copyright notice in the Description page of Project Settings.


#include "RankingSubsystem.h"
#include "Dom/JsonObject.h"						//Json 객체를 사용하기 위한 헤더
#include "Dom/JsonValue.h"						//Json 배열 값을 사용하기 위한 헤더
#include "Serialization/JsonReader.h"			//Json 문자열을 읽기 위한 헤더
#include "Serialization/JsonSerializer.h"		//Json 직렬화와 역직렬화를 위한 헤더
#include "Serialization/JsonWriter.h"			//Json 문자열을 쓰기위한 헤더

//FJsonObject, FJsonValue, TJsonReader
//Json 객체 하나를 FGameResult 구조체로 변환하는 내부용 함수
static void JsonObjectToGameResult(const TSharedPtr<FJsonObject>& JsonObject, OUT FGameResult& OutResult)
{
	if (!JsonObject.IsValid())  //Json 객체가 유효하지 않으면 처리 안함
		return;
	
	//Json의 숫자 값은 Double로 읽히기 때문에 만든 임시 변환 변수
	double IdValue = 0.0;	
	//Json 문자열에서 key 값이 "id"이거나 "Id"인 애를 찾으면 IdValue 변수에 해당 Value를 넣어줘
	if (JsonObject->TryGetNumberField(TEXT("id"), IdValue) || JsonObject->TryGetNumberField(TEXT("Id"), IdValue))
		OutResult.Id = static_cast<int32>(IdValue);

	double UserIdValue = 0.0;
	if (JsonObject->TryGetNumberField(TEXT("userId"), UserIdValue) || JsonObject->TryGetNumberField(TEXT("UserId"), UserIdValue))
		OutResult.UserId = static_cast<int32>(UserIdValue);

	if (JsonObject->TryGetStringField(TEXT("userName"), OutResult.UserName) || JsonObject->TryGetStringField(TEXT("UserName"), OutResult.UserName))
	{
		//필요하면 if문 사용하기
	}	

	if (JsonObject->TryGetStringField(TEXT("userEmail"), OutResult.UserEmail) || JsonObject->TryGetStringField(TEXT("UserEmail"), OutResult.UserEmail))
	{
		//필요하면 if문 사용하기
	}

	double ScoreValue = 0.0;
	if (JsonObject->TryGetNumberField(TEXT("score"), ScoreValue) || JsonObject->TryGetNumberField(TEXT("Score"), ScoreValue))
		OutResult.Score = static_cast<int32>(ScoreValue);

	if (JsonObject->TryGetStringField(TEXT("date"), OutResult.Date) || JsonObject->TryGetStringField(TEXT("Date"), OutResult.Date))
	{
		//필요하면 if문 사용하기
	}
}

//FGameResult를 Json 문자열로 바꾸는 함수
static FString MakeJsonFromGameResult(const FGameResult& Result)
{
	//새로운 Json 객체를 생성 -> 스마트포인터를 통해서 참조가 끝나면 메모리 관리를 알아서 해줌, delete 챙길 필요 없음, 참조가 없으면 메모리 정리됨
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	//id라는 필드에 GameResult.Id 값을 넣기
	JsonObject->SetNumberField(TEXT("id"), Result.Id);

	//userId라는 필드에 GameResult.UserId 값을 넣기
	JsonObject->SetNumberField(TEXT("userid"), Result.UserId);

	//UserName이라는 필드에 GameResult.UserName 값을 넣기
	JsonObject->SetStringField(TEXT("username"), Result.UserName);

	//UserEmail이라는 필드에 GameResult.UserEmail 값을 넣기
	JsonObject->SetStringField(TEXT("userEmail"), Result.UserEmail);

	//score라는 필드에 GameResult.Score 값을 넣기
	JsonObject->SetNumberField(TEXT("score"), Result.Score);

	//date라는 필드에 GameResult.Date 값을 넣기
	JsonObject->SetStringField(TEXT("date"), Result.Date);

	//Json 문자열로 변환하고 담아둘 임시 변수
	FString JsonString;

	//FString에 Json을 쓰는 Writer를 생성
	TSharedPtr<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);

	//Json 객체를 문자열로 변환
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), *Writer);
	
	return JsonString;
}

//랭킹 데이터를 서버에 추가하는 함수
void URankingSubsystem::AddGameResult(FGameResult NewResult)
{
	//Date 값 언리얼 말고 C++에서 채워주는 용도
	if (NewResult.Date.IsEmpty())
		NewResult.Date = FDateTime::UtcNow().ToIso8601();  //Date가 비어있으면 UTC 시간을 ISO 형식으로 넣기
	
	//서버로 보낼 JsonBody 만들기
	FString JsonBody = MakeJsonFromGameResult(NewResult);

	//HTTP 요청 객체를 생성
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	//POST 요청을 보낼 URL(엔드포인트) 설정
	Request->SetURL(BaseUrl);

	//요청을 보낼 Verb 설정
	Request->SetVerb(TEXT("POST"));

	//요청을 보낼 때 내가 어떤 타입의 데이터를 보내는지 설정
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	//요청에 BodyJson 넣게
	Request->SetContentAsString(JsonBody);

	//요청 완료(응답) 시 실행할 콜백 함수 연결
	Request->OnProcessRequestComplete().BindUObject(this, &URankingSubsystem::OnAddGameResultResponse);

	//HTTP 요청을 서버로 전송
	Request->ProcessRequest();
}

//전체 랭킹 리스트를 서버에서 가져오는 함수
void URankingSubsystem::GetGameResults()
{
	//HTTP 요청 객체를 생성
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	//POST 요청을 보낼 URL(엔드포인트) 설정
	Request->SetURL(BaseUrl);

	//요청을 보낼 Verb 설정
	Request->SetVerb(TEXT("GET"));

	//요청을 보낼 때 내가 어떤 타입의 데이터를 보내는지 설정
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	//요청 완료(응답) 시 실행할 콜백 함수 연결
	Request->OnProcessRequestComplete().BindUObject(this, &URankingSubsystem::OnGetGameResultsResponse);

	//HTTP 요청을 서버로 전송
	Request->ProcessRequest();
}

//단일 랭킹을 서버에서 가져오는 함수
void URankingSubsystem::GetGameResult(int32 TargetId)
{
	//쿼리파라미터가 담긴 URL 생성
	FString Url = BaseUrl + TEXT("/") + FString::FromInt(TargetId);

	//HTTP 요청 객체를 생성
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	//POST 요청을 보낼 URL(엔드포인트) 설정
	Request->SetURL(Url);

	//요청을 보낼 Verb 설정
	Request->SetVerb(TEXT("GET"));

	//요청을 보낼 때 내가 어떤 타입의 데이터를 보내는지 설정
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	//요청 완료(응답) 시 실행할 콜백 함수 연결
	Request->OnProcessRequestComplete().BindUObject(this, &URankingSubsystem::OnGetGameResultResponse);

	//HTTP 요청을 서버로 전송
	Request->ProcessRequest();
}

//서버에 있는 랭킹 데이터를 수정 요청하는 함수
void URankingSubsystem::UpdateGameResult(FGameResult TargetResult)
{
	//Date 값 언리얼 말고 C++에서 채워주는 용도
	if (TargetResult.Date.IsEmpty())
		TargetResult.Date = FDateTime::UtcNow().ToIso8601();  //Date가 비어있으면 UTC 시간을 ISO 형식으로 넣기

	//서버로 보낼 JsonBody 만들기
	FString JsonBody = MakeJsonFromGameResult(TargetResult);

	//HTTP 요청 객체를 생성
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	//POST 요청을 보낼 URL(엔드포인트) 설정
	Request->SetURL(BaseUrl);

	//요청을 보낼 Verb 설정
	Request->SetVerb(TEXT("PUT"));

	//요청을 보낼 때 내가 어떤 타입의 데이터를 보내는지 설정
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	//요청에 BodyJson 넣게
	Request->SetContentAsString(JsonBody);

	//요청 완료(응답) 시 실행할 콜백 함수 연결
	Request->OnProcessRequestComplete().BindUObject(this, &URankingSubsystem::OnUpdateGameResultResponse);

	//HTTP 요청을 서버로 전송
	Request->ProcessRequest();
}

void URankingSubsystem::DeleteGameResult(int32 TargetId)
{
	//쿼리파라미터가 담긴 URL 생성
	FString Url = BaseUrl + TEXT("/") + FString::FromInt(TargetId);

	//HTTP 요청 객체를 생성
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	//POST 요청을 보낼 URL(엔드포인트) 설정
	Request->SetURL(Url);

	//요청을 보낼 Verb 설정
	Request->SetVerb(TEXT("DELETE"));

	//요청을 보낼 때 내가 어떤 타입의 데이터를 보내는지 설정
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	//요청 완료(응답) 시 실행할 콜백 함수 연결
	Request->OnProcessRequestComplete().BindUObject(this, &URankingSubsystem::OnDeleteGameResultResponse);

	//HTTP 요청을 서버로 전송
	Request->ProcessRequest();
}

//Add 요청을 보내고 응답을 처리하는 콜백 함수
void URankingSubsystem::OnAddGameResultResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	//서버 연결에 실패했거나, 응답이 유효하지 않은지 확인
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		//요청이 실패했거나, 유효하지 않다면 실패 했다고 방송
		OnActionCompleted.Broadcast(false, TEXT("POST"));
		return;
	}
	
	//응답 객체로부터 콘텐츠(결과값을 문자열로) 가져오기
	FString ResponseString = Response->GetContentAsString();
	//결과값 문자열이 대소문자 구분없이 "true"라면 bool == true;
	bool bSuccess = ResponseString.Equals(TEXT("true"), ESearchCase::IgnoreCase);

	//성공/실패 여부 방송
	OnActionCompleted.Broadcast(bSuccess, TEXT("POST"));
}

//Get(전체) 요청을 보내고 응답을 처리하는 콜백 함수
void URankingSubsystem::OnGetGameResultsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	//블루프린트로 방송할 랭킹 임시 배열
	TArray<FGameResult> RankingList;

	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		//요청이 실패했거나, 유효하지 않다면 실패 했다고 방송
		OnGetRankingCompleted.Broadcast(RankingList);
		return;
	}

	//응답 객체로부터 콘텐츠(결과값을 문자열로) 가져오기
	FString ResponseString = Response->GetContentAsString();
		
	//Json 배열 데이터를 저장할 배열
	TArray<TSharedPtr<FJsonValue>> JsonArray;

	//Json 문자열을 읽는 Reader 객체 생성
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

	//Json 문자열을 Json 배열로 변환
	bool bParseSuccess = FJsonSerializer::Deserialize(Reader, JsonArray);

	//Json 배열로 만드는게 실패했다면
	if (!bParseSuccess)
	{
		OnGetRankingCompleted.Broadcast(RankingList);
		return;
	}

	//Json 배열에서 Json Object 하나씩 꺼내기
	for (auto JsonValue : JsonArray)
	{
		auto JsonObject = JsonValue->AsObject();
		
		//Json 오브젝트를 게임 구조체로 변경한걸 담을 임시 변수
		FGameResult Result;
		//Json 오브젝트를 게임 구조체로 변경
		JsonObjectToGameResult(JsonObject, Result);
		//반환할 배열 변수에 담아주기
		RankingList.Add(Result);
	}

	//완성된 랭킹 배열을 방송
	OnGetRankingCompleted.Broadcast(RankingList);
}

//Get(전체) 요청을 보내고 응담을 처리하는 콜백 함수
void URankingSubsystem::OnGetGameResultResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	FGameResult Result;

	//서버 연결에 실패 했거나, 응답이 유효하지 않은지 확인
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		//요청이 실패했거나, 유효하지 않다면 실패 했다고 방송
		OnGetSingleRankingCompleted.Broadcast(false, Result);
		return;
	}

	//응답 객체로부터 콘텐츠(결과값을 문자열로) 가져오기
	FString ResponseString = Response->GetContentAsString();
	
	//응답이 비어있거나, null 이라는게 왔는지 확인
	if (ResponseString.IsEmpty() || ResponseString.Equals(TEXT("null"), ESearchCase::IgnoreCase))
	{
		OnGetSingleRankingCompleted.Broadcast(false, Result);
		return;
	}

	//FGameResult로 변환할 Json 객체를 임시로 담아둘 변수
	TSharedPtr<FJsonObject> JsonObject;

	//Json 문자열을 읽는 Reader 객체 생성
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

	//Json 문자열을 Json 배열로 변환
	bool bParseSuccess = FJsonSerializer::Deserialize(Reader, JsonObject);

	if (!bParseSuccess || !JsonObject.IsValid())
	{
		OnGetSingleRankingCompleted.Broadcast(false, Result);
		return;
	}

	//Json 오브젝트를 게임 구조체로 변경
	JsonObjectToGameResult(JsonObject, Result);

	//Json 변환이 성공했다는 것과 FGameResult 방송
	OnGetSingleRankingCompleted.Broadcast(true, Result);
}

//Get(단일) 요청을 보내고 응담을 처리하는 콜백 함수
void URankingSubsystem::OnUpdateGameResultResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	//서버 연결에 실패했거나, 응답이 유효하지 않은지 확인
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		//요청이 실패했거나, 유효하지 않다면 실패 했다고 방송
		OnActionCompleted.Broadcast(false, TEXT("PUT"));
		return;
	}

	//응답 객체로부터 콘텐츠(결과값을 문자열로) 가져오기
	FString ResponseString = Response->GetContentAsString();
	//결과값 문자열이 대소문자 구분없이 "true"라면 bool == true;
	bool bSuccess = ResponseString.Equals(TEXT("true"), ESearchCase::IgnoreCase);

	//성공/실패 여부 방송
	OnActionCompleted.Broadcast(bSuccess, TEXT("PUT"));
}

//Delete 요청을 보내고 응담을 처리하는 콜백 함수
void URankingSubsystem::OnDeleteGameResultResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	//서버 연결에 실패했거나, 응답이 유효하지 않은지 확인
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		//요청이 실패했거나, 유효하지 않다면 실패 했다고 방송
		OnActionCompleted.Broadcast(false, TEXT("DELETE"));
		return;
	}

	//응답 객체로부터 콘텐츠(결과값을 문자열로) 가져오기
	FString ResponseString = Response->GetContentAsString();
	//결과값 문자열이 대소문자 구분없이 "true"라면 bool == true;
	bool bSuccess = ResponseString.Equals(TEXT("true"), ESearchCase::IgnoreCase);

	//성공/실패 여부 방송
	OnActionCompleted.Broadcast(bSuccess, TEXT("DELETE"));
}
