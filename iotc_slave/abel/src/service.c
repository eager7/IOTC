/****************************************************************************
 *
 * MODULE:             service.c
 *
 * COMPONENT:          Utils interface
 *
 * REVISION:           $Revision:  0$
 *
 * DATED:              $Date: 2015-10-21 15:13:17 +0100 (Thu, 21 Oct 2015 $
 *
 * AUTHOR:             Abel
 *
 ****************************************************************************
 *
 * Copyright IOTC project B.V. 2015. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <json/json.h>

#include "utils.h"



/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define TYPE_STATEPOST		"StatePost"
#define TYPE_SERVICEPOST	"ServiceReq"


/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/



/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/




/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/



/****************************************************************************/
/***        Local    Functions                                            ***/
/****************************************************************************/


json_object *IoctMsgNew(const char *type)
{
	if(!type){
		ERR_vPrintf(T_TRUE, "IoctMsgNew error\n");
        return NULL;
	}

	json_object *root;

	root = json_object_new_object();
    if(NULL == root)
    {
        ERR_vPrintf(T_TRUE, "IoctMsgNew error\n");
        return NULL;
    }

	json_object_object_add(root, "MsgType", json_object_new_string(type));

	return root;
}


void IoctMsgDel(json_object *obj)
{
	if(!obj){
		WARN_vPrintf(T_TRUE, "IoctMsgDel Warnning: Obj NULL\n");
	}
	else{
		json_object_put(obj);
	}

	return;
}



json_object *StatePostIoctMsgInit(char *DevNum)
{
	if(!DevNum){
		ERR_vPrintf(T_TRUE, "StatePostIoctMsgInit error\n");
        return ;
	}
	json_object *root;

	root = IoctMsgNew(TYPE_STATEPOST);
	if(!root){
		ERR_vPrintf(T_TRUE, "StatePostIoctMsgInit error\n");
        return NULL;
	}
	
	json_object_object_add(root, "DevNum", json_object_new_string(DevNum));

	json_object *array;
	
	array = json_object_new_array();
	if(!root){
		ERR_vPrintf(T_TRUE, "StatePostIoctMsgInit error\n");
		json_object_put(root);
        return NULL;
	}

	json_object_object_add(root, "StateList", array);

	return root;
}



json_object *ServiceReqIoctMsgInit(char *DevNum)
{
	if(!DevNum){
		ERR_vPrintf(T_TRUE, "StatePostIoctMsgInit error\n");
        return ;
	}
	json_object *root;

	root = IoctMsgNew(TYPE_STATEPOST);
	if(!root){
		ERR_vPrintf(T_TRUE, "StatePostIoctMsgInit error\n");
        return NULL;
	}
	
	json_object_object_add(root, "name", json_object_new_string(DevNum));

	json_object *array;
	
	array = json_object_new_array();
	if(!root){
		ERR_vPrintf(T_TRUE, "StatePostIoctMsgInit error\n");
		json_object_put(root);
        return NULL;
	}

	json_object_object_add(root, "StateList", array);

	return root;
}


Descriptor *DescriptorNew()
{
	Descriptor *des;

	des = (Descriptor *)malloc(sizeof(Descriptor));
	if(!des){
		ERR_vPrintf(T_TRUE, "DescriptorNew error\n");
        return NULL;
	}

	des->url= NULL;
	des->descriptor = NULL;
	dl_list_init(&(des->list));

	return des;
}


eIoctProtolStatus DescriptorSetUrl(Descriptor *des, char *url)
{
	if(!des){
		ERR_vPrintf(T_TRUE, "DescriptorSetUrl error: des is Null\n");
        return E_IOCTP_ARG_ERROR;
	}

	if(!url){
		ERR_vPrintf(T_TRUE, "DescriptorSetUrl error: url is Null\n");
		return E_IOCTP_ARG_ERROR;
	}

	int len;

	len = strlen(url);

	if(des->url){
		free(des->url);
	}

	des->url = (char *)malloc(len + 1);
	if(!(des->url)){
		ERR_vPrintf(T_TRUE, "DescriptorSetUrl error: malloc failed\n");
		return E_IOCTP_ERROR;
	}

	memset(des->url, 0, len+1);
	memcpy(des->url, url, len);

	return E_IOCTP_OK;

	
}



eIoctProtolStatus DescriptorGetUrl(Descriptor *des, char *url)
{
	if(!des){
		ERR_vPrintf(T_TRUE, "DescriptorSetUrl error: des is Null\n");
        return E_IOCTP_ARG_ERROR;
	}

	if(!url){
		ERR_vPrintf(T_TRUE, "DescriptorSetUrl error: url is Null\n");
		return E_IOCTP_ARG_ERROR;
	}

	int len;

	len = strlen(url);

	if(des->url){
		free(des->url);
	}

	des->url = (char *)malloc(len + 1);
	if(!(des->url)){
		ERR_vPrintf(T_TRUE, "DescriptorSetUrl error: malloc failed\n");
		return E_IOCTP_ERROR;
	}

	memset(des->url, 0, len+1);
	memcpy(des->url, url, len);

	return E_IOCTP_OK;

	
}



eIoctProtolStatus DescriptorListAddNode(DescriptorList *list, Descriptor *Node)
{
	if(!list){
		ERR_vPrintf(T_TRUE, "DescriptorListAddNode error\n");
        return E_IOCTP_ARG_ERROR;
	}

	if(!Node){
		WARN_vPrintf(T_TRUE, "DescriptorListAddNode Warnning: Node is Null\n");
        return E_IOCTP_OK;
	}

	dl_list_add_tail(&(list->list), &(Node->list));

	return E_IOCTP_OK;
}


void DescriptorDel(Descriptor *des)
{
	if(!des){
		WARN_vPrintf(T_TRUE, "DescriptorDel Warnning: des NULL\n");
		return;
	}
	
	dl_list_del(&(des->list));
	if(des->url){
		free(des->url);
	}
	if(des->descriptor){
		free(des->descriptor);
	}

	free(des);
	
	return;
}


eIoctProtolStatus DescriptorListDelNode(IotcDeviceList *list, char *mName)
{
	if(!list){
		ERR_vPrintf(T_TRUE, "IotcDeviceAddNode error\n");
        return E_IOCTP_ARG_ERROR;
	}

	if(!mName){
		ERR_vPrintf(T_TRUE, "IotcDeviceAddNode error\n");
        return E_IOCTP_ARG_ERROR;
	}

	IotcDevice *des1 = NULL;
	IotcDevice *des2 = NULL;
	dl_list_for_each_safe(des1, des2, &(list->list), IotcDevice, list);
	{
		if(des1){
			if(!strcmp(des1.mName, mName)){
				IotcDeviceDel(des1);
			}
			else
				continue;
		}
		
	}
	return E_IOCTP_OK;
}



IotcAttribute *IotcAttributeNew()
{
	IotcAttribute *attr;

	attr = (IotcAttribute *)malloc(sizeof(IotcAttribute));
	if(!attr){
		ERR_vPrintf(T_TRUE, "IotcAttribute error\n");
        return NULL;
	}

	attr->name = NULL;
	attr->value = NULL;
	dl_list_init(&(attr->list));

	return attr;
}


eIoctProtolStatus IotcAttributeListAddNode(IotcAttributeList *list, IotcAttribute *Node)
{
	if(!list){
		ERR_vPrintf(T_TRUE, "IotcAttributeListAddNode error\n");
        return E_IOCTP_ARG_ERROR;
	}

	if(!Node){
		WARN_vPrintf(T_TRUE, "IotcAttributeListAddNode Warnning: Node is Null\n");
        return E_IOCTP_OK;
	}

	dl_list_add_tail(&(list->list), &(Node->list));

	return E_IOCTP_OK;
}


void IotcAttributeDel(IotcAttribute *attr)
{
	if(!attr){
		WARN_vPrintf(T_TRUE, "IotcAttributeDel Warnning: attr NULL\n");
		return;
	}
	
	dl_list_del(&(attr->list));
	if(attr->name){
		free(attr->name);
	}
	if(attr->value){
		free(attr->value);
	}
	
	free(attr);
	
	return;
}


IotcArgument *IotcArgumentNew()
{
	IotcArgument *arg;

	arg = (IotcArgument *)malloc(sizeof(IotcArgument));
	if(!arg){
		ERR_vPrintf(T_TRUE, "IotcArgumentNew error\n");
        return NULL;
	}

	arg->name = NULL;
	arg->value = NULL;
	dl_list_init(&(arg->list));

	return arg;
}


eIoctProtolStatus IotcArgumentListAddNode(IotcArgumentList *list, IotcArgument *Node)
{
	if(!list){
		ERR_vPrintf(T_TRUE, "IotcArgumentListAddNode error\n");
        return E_IOCTP_ARG_ERROR;
	}

	if(!Node){
		WARN_vPrintf(T_TRUE, "IotcArgumentListAddNode Warnning: Node is Null\n");
        return E_IOCTP_OK;
	}

	dl_list_add_tail(&(list->list), &(Node->list));

	return E_IOCTP_OK;
}



void IotcArgumentDel(IotcArgument *arg)
{
	if(!arg){
		WARN_vPrintf(T_TRUE, "IotcArgumentDel Warnning: arg NULL\n");
		return;
	}

	dl_list_del(&(arg->list));
	if(arg->name){
		free(arg->name);
	}
	if(arg->value){
		free(arg->value);
	}

	free(arg);
	
	return;
}



IotcService *IotcServiceNew()
{
	IotcService *ser;

	ser = (IotcService *)malloc(sizeof(IotcService));
	if(!ser){
		ERR_vPrintf(T_TRUE, "IotcServiceNew error\n");
        return NULL;
	}

	ser->name = NULL;
	dl_list_init(&(ser->list));

	return ser;
}


eIoctProtolStatus IotcServiceListNode(IotcServiceList *list, IotcService *Node)
{
	if(!list){
		ERR_vPrintf(T_TRUE, "IotcServiceListNode error\n");
        return E_IOCTP_ARG_ERROR;
	}

	if(!Node){
		WARN_vPrintf(T_TRUE, "IotcServiceListNode Warnning: Node is Null\n");
        return E_IOCTP_OK;
	}

	dl_list_add_tail(&(list->list), &(Node->list));

	return E_IOCTP_OK;
}



void IotcServiceDel(IotcService *ser)
{
	IotcArgumentList *tmp1 = NULL;
	IotcArgumentList *tmp2 = NULL;

	if(!ser){
		WARN_vPrintf(T_TRUE, "IotcServiceDel Warnning: ser NULL\n");
		return;
	}
	else{
		dl_list_del(&(ser->list));
		if(ser->name){
			free(ser->name);
		}

		dl_list_for_each_safe(tmp1, tmp2, &((ser->argList).list), IotcArgumentList, list);
		{
			IotcArgumentDel(tmp1);
		}

		free(ser);
	}
	
	return;
}


IotcDevice *IotcDeviceNew()
{
	IotcDevice *dev;

	dev = (IotcDevice *)malloc(sizeof(IotcDevice));
	if(!dev){
		ERR_vPrintf(T_TRUE, "IotcDeviceNew error\n");
        return NULL;
	}

	dev->name = NULL;
	dev->mName = NULL;
	dev->DevNum = NULL;
	
	dl_list_init(&dev->list);

	return dev;
}


eIoctProtolStatus IotcDeviceListAddNode(IotcDeviceList *list, IotcDevice *Node)
{
	if(!list){
		ERR_vPrintf(T_TRUE, "IotcDeviceListAddNode error\n");
        return E_IOCTP_ARG_ERROR;
	}

	if(!Node){
		WARN_vPrintf(T_TRUE, "IotcDeviceListAddNode Warnning: Node is Null\n");
        return E_IOCTP_OK;
	}

	dl_list_add_tail(&(list->list), &(Node->list));

	return E_IOCTP_OK;
}



void IotcDeviceDel(IotcDevice *dev)
{
	if(!dev){
		WARN_vPrintf(T_TRUE, "IotcDeviceDel Warnning: dev NULL\n");
		return;
	}

	dl_list_del(&dev.list);

	if(dev->name){
		free(dev->name);
	}
	if(dev->DevNum){
		free(dev->DevNum);
	}
	if(dev->mName){
		free(dev->mName);
	}

	IotcAttribute *attr1 = NULL;
	IotcAttribute *attr2 = NULL;
	dl_list_for_each_safe(attr1, attr2, &((dev->attrList).list), IotcAttribute, list);
	{
		IotcAttributeDel(attr1);
	}

	IotcService *ser1 = NULL;
	IotcService *ser2 = NULL;
	dl_list_for_each_safe(ser1, ser2, &((dev->serList).list), IotcService, list);
	{
		IotcServiceDel(ser1);
	}

	free(dev);
	
	return;
}


eIoctProtolStatus IotcDeviceListDelNode(IotcDeviceList *list, char *DevNum)
{
	if(!list){
		ERR_vPrintf(T_TRUE, "IotcDeviceAddNode error\n");
        return E_IOCTP_ARG_ERROR;
	}

	if(!DevNum){
		ERR_vPrintf(T_TRUE, "IotcDeviceAddNode error\n");
        return E_IOCTP_ARG_ERROR;
	}

	IotcDevice *dev1 = NULL;
	IotcDevice *dev2 = NULL;
	dl_list_for_each_safe(dev1, dev2, &(list->list), IotcDevice, list);
	{
		if(dev1){
			if(!strcmp(dev1.DevNum, DevNum)){
				IotcDeviceDel(dev1);
			}
			else
				continue;
		}
		
	}
	return E_IOCTP_OK;
}




//get and parse descriptor
eIoctProtolStatus GPDescriptorFromUrl(char *url, Descriptor *desList, IotcDeviceList *devList)
{
	bool_t ret = T_FALSE;
	if((!url)||(!desList)){
		ERR_vPrintf(T_TRUE, "GetDescriptorFromUrl error\n");
        return E_IOCTP_ARG_ERROR;
	}

	//1. search url from deslist
	Descriptor *des1 = NULL;
	Descriptor *des2 = NULL;
	dl_list_for_each_safe(des1, des2, &(desList->list), IotcDevice, list);
	{
		if(des1){
			if(!strcmp(des1->url, url)){
				ret = T_TRUE;
				break;
			}
			else
				continue;
		}
		
	}

	char *realDes = NULL;
	Descriptor *des = NULL;
	
	//2. get descriptor from url and add desnode
	if(T_TRUE == ret){   //descriptor exist, didn't got it again, didn't add node again
		realDes = des1->descriptor;
	}
	else{   //would get descriptor, step by new struct Descriptor, get context , assignment 
		des = httpget(url);//get url
	}

	
	//4. parse descriptor
	DescriptorParse(realDes, devList)

}




eIoctProtolStatus DescriptorParse(char *des, IotcDeviceList *devList)
{
	if((!des)||(!devList)){
		ERR_vPrintf(T_TRUE, "DescriptorParse error\n");
        return E_IOCTP_ARG_ERROR;
	}

	//1. parse des string to json
	
	//2. allot DevNum

	//3. creat IotcDevice node

	//4. add node to Device list

}

