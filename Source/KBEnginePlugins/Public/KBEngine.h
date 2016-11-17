// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

class KBEngineArgs;
class Entity;
class NetworkInterface;
class MemoryStream;
class PersistentInfos;

/*
	这是KBEngine插件的核心模块
	包括网络创建、持久化协议、entities的管理、以及引起对外可调用接口。

	一些可以参考的地方:
	http://www.kbengine.org/docs/programming/clientsdkprogramming.html
	http://www.kbengine.org/docs/programming/kbe_message_format.html
*/
class KBENGINEPLUGINS_API KBEngineApp : public InterfaceLogin
{
public:
	KBEngineApp();
	KBEngineApp(KBEngineArgs* pArgs);
	virtual ~KBEngineApp();
	
public:
	static KBEngineApp& getSingleton();

public:
	bool isInitialized() const {
		return pArgs_ != NULL;
	}

	bool initialize(KBEngineArgs* pArgs);
	void destroy();
	void reset();

	NetworkInterface* pNetworkInterface() const {
		return pNetworkInterface_;
	}

	const TArray<uint8>& serverdatas() const
	{
		return serverdatas_;
	}

	KBEngineArgs* getInitArgs() const
	{
		return pArgs_;
	}

	void installEvents();

	void resetMessages();

	static bool validEmail(FString strEmail);

	/*
		通过错误id得到错误描述
	*/
	FString serverErr(uint16 id);

	Entity* player();
	Entity* findEntity(int32 entityID);

	/**
		插件的主循环处理函数
	*/
	void process();

	/*
		向服务端发送心跳以及同步角色信息到服务端
	*/
	void sendTick();

	/**
		登录到服务端，必须登录完成loginapp与网关(baseapp)，登录流程才算完毕
	*/
	bool login(FString& username, FString& password, TArray<uint8>& datas);
	virtual void onLoginCallback(FString ip, uint16 port, bool success, int userdata) override;

	/*
	重登录到网关(baseapp)
	一些移动类应用容易掉线，可以使用该功能快速的重新与服务端建立通信
	*/
	void reLoginBaseapp();

	/*
		登录loginapp失败了
	*/
	void Client_onLoginFailed(MemoryStream& stream);

	/*
		登录loginapp成功了
	*/
	void Client_onLoginSuccessfully(MemoryStream& stream);

	/*
		登录baseapp失败了
	*/
	void Client_onLoginBaseappFailed(uint16 failedcode);

	/*
		重登录baseapp失败了
	*/
	void Client_onReLoginBaseappFailed(uint16 failedcode);

	/*
		登录baseapp成功了
	*/
	void Client_onReLoginBaseappSuccessfully(MemoryStream& stream);

	void hello();
	void Client_onHelloCB(MemoryStream& stream);

	void Client_onVersionNotMatch(MemoryStream& stream);
	void Client_onScriptVersionNotMatch(MemoryStream& stream);

	/*
		被服务端踢出
	*/
	void Client_onKicked(uint16 failedcode);

	/*
	从服务端返回的二进制流导入客户端消息协议
	*/
	void Client_onImportClientMessages(MemoryStream& stream);

	/*
		服务端错误描述导入了
	*/
	void Client_onImportServerErrorsDescr(MemoryStream& stream);

	void Client_onImportClientEntityDef(MemoryStream& stream);

	/*
		服务器心跳回调
	*/
	void Client_onAppActiveTickCB();

	/*
		服务端通知创建一个角色
	*/
	void Client_onCreatedProxies(uint64 rndUUID, int32 eid, FString& entityType);

	/*
		服务端通知强制销毁一个实体
	*/
	void Client_onEntityDestroyed(int32 eid);

	/*
		服务端使用优化的方式更新实体属性数据
	*/
	void Client_onUpdatePropertysOptimized(MemoryStream& stream);

	/*
		服务端更新实体属性数据
	*/
	void Client_onUpdatePropertys(MemoryStream& stream);

	/*
		服务端更新实体属性数据
	*/
	void onUpdatePropertys_(ENTITY_ID eid, MemoryStream& stream);

private:
	bool initNetwork();

	void login_loginapp(bool noconnect);
	void onConnectTo_loginapp_callback(FString ip, uint16 port, bool success);
	void onLogin_loginapp();

	void login_baseapp(bool noconnect);
	void onConnectTo_baseapp_callback(FString ip, uint16 port, bool success);
	void onLogin_baseapp();


	void clearSpace(bool isall);
	void clearEntities(bool isall);


	void updatePlayerToServer();

	void onServerDigest();

	/*
		从二进制流导入消息协议完毕了
	*/
	void onImportClientMessagesCompleted();
	void onImportEntityDefCompleted();
	void onImportClientMessages(MemoryStream& stream);
	void onImportServerErrorsDescr(MemoryStream& stream);

	void createDataTypeFromStreams(MemoryStream& stream, bool canprint);
	void createDataTypeFromStream(MemoryStream& stream, bool canprint);
	void onImportClientEntityDef(MemoryStream& stream);

	void resetpassword_loginapp(bool noconnect);

	void createAccount_loginapp(bool noconnect);

	/*
		通过流数据获得AOI实体的ID
	*/
	ENTITY_ID getAoiEntityIDFromStream(MemoryStream& stream);

public:
	ENTITY_ID entity_id() const {
		return entity_id_;
	}

	uint64 entity_uuid() const {
		return entity_uuid_;
	}

	const FString& entity_type() const {
		return entity_type_;
	}



protected:
	KBEngineArgs* pArgs_;
	NetworkInterface* pNetworkInterface_;

	FString username_;
	FString password_;

	// 是否正在加载本地消息协议
	static bool loadingLocalMessages_;

	// 消息协议是否已经导入了
	static bool loginappMessageImported_;
	static bool baseappMessageImported_;
	static bool entitydefImported_;
	static bool isImportServerErrorsDescr_;

	// 服务端分配的baseapp地址
	FString baseappIP_;
	uint16 baseappPort_;

	// 当前状态
	FString currserver_;
	FString currstate_;

	// 服务端下行以及客户端上行用于登录时处理的账号绑定的二进制信息
	// 该信息由用户自己进行扩展
	TArray<uint8> serverdatas_;
	TArray<uint8> clientdatas_;

	// 通信协议加密，blowfish协议
	TArray<uint8> encryptedKey_;

	// 服务端与客户端的版本号以及协议MD5
	FString serverVersion_;
	FString clientVersion_;
	FString serverScriptVersion_;
	FString clientScriptVersion_;
	FString serverProtocolMD5_;
	FString serverEntitydefMD5_;

	// 持久化插件信息， 例如：从服务端导入的协议可以持久化到本地，下次登录版本不发生改变
	// 可以直接从本地加载来提供登录速度
	PersistentInfos* persistentInfos_;

	// 当前玩家的实体id与实体类别
	uint64 entity_uuid_;
	ENTITY_ID entity_id_;
	FString entity_type_;

	// space的数据，具体看API手册关于spaceData
	// https://github.com/kbengine/kbengine/tree/master/docs/api
	TMap<FString, FString> spacedatas_;
	
	// 所有实体都保存于这里， 请参看API手册关于entities部分
	// https://github.com/kbengine/kbengine/tree/master/docs/api
	TMap<ENTITY_ID, Entity*> entities_;

	// 在玩家AOI范围小于256个实体时我们可以通过一字节索引来找到entity
	TArray<ENTITY_ID> entityIDAliasIDList_;
	TMap<ENTITY_ID, MemoryStream*> bufferedCreateEntityMessage_;

	// 所有服务端错误码对应的错误描述
	static TMap<uint16, FKServerErr> serverErrs_;

	double lastTickTime_;
	double lastTickCBTime_;
	double lastUpdateToServerTime_;

	// 玩家当前所在空间的id， 以及空间对应的资源
	SPACE_ID spaceID_;
	FString spaceResPath_;
	bool isLoadedGeometry_;

	// 按照标准，每个客户端部分都应该包含这个属性
	FString component_;

};

