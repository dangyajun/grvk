#include "mantle_internal.h"

// Generic API Object Management functions

GR_RESULT grGetObjectInfo(
    GR_BASE_OBJECT object,
    GR_ENUM infoType,
    GR_SIZE* pDataSize,
    GR_VOID* pData)
{
    LOGT("%p 0x%X %p %p\n", object, infoType, pDataSize, pData);
    GrBaseObject* grBaseObject = (GrBaseObject*)object;

    if (grBaseObject == NULL) {
        return GR_ERROR_INVALID_HANDLE;
    } else if (pDataSize == NULL) {
        return GR_ERROR_INVALID_POINTER;
    }

    GrObjectType objType = GET_OBJ_TYPE(grBaseObject);

    switch (infoType) {
    case GR_INFO_TYPE_MEMORY_REQUIREMENTS: {
        GR_MEMORY_REQUIREMENTS* grMemReqs = (GR_MEMORY_REQUIREMENTS*)pData;
        VkMemoryRequirements memReqs;

        if (pData == NULL) {
            *pDataSize = sizeof(GR_MEMORY_REQUIREMENTS);
            return GR_SUCCESS;
        } else if (*pDataSize != sizeof(GR_MEMORY_REQUIREMENTS)) {
            return GR_ERROR_INVALID_MEMORY_SIZE;
        }

        if (objType == GR_OBJ_TYPE_IMAGE) {
            GrImage* grImage = (GrImage*)grBaseObject;
            GrDevice* grDevice = GET_OBJ_DEVICE(grBaseObject);

            VKD.vkGetImageMemoryRequirements(grDevice->device, grImage->image, &memReqs);

            *grMemReqs = getGrMemoryRequirements(memReqs);
        } else if (objType == GR_OBJ_TYPE_DESCRIPTOR_SET ||
                   objType == GR_OBJ_TYPE_PIPELINE) {
            // No memory requirements
            *grMemReqs = (GR_MEMORY_REQUIREMENTS) {
                .size = 4,
                .alignment = 4,
                .heapCount = 0,
            };
        } else {
            LOGW("unsupported type %d for info type 0x%X\n", objType, infoType);
            return GR_ERROR_INVALID_VALUE;
        }
    }   break;
    default:
        LOGW("unsupported info type 0x%X\n", infoType);
        return GR_ERROR_INVALID_VALUE;
    }

    return GR_SUCCESS;
}

GR_RESULT grBindObjectMemory(
    GR_OBJECT object,
    GR_GPU_MEMORY mem,
    GR_GPU_SIZE offset)
{
    LOGT("%p %p %llu\n", object, mem, offset);
    GrObject* grObject = (GrObject*)object;
    GrGpuMemory* grGpuMemory = (GrGpuMemory*)mem;
    VkResult vkRes = VK_SUCCESS;

    if (grObject == NULL) {
        return GR_ERROR_INVALID_HANDLE;
    }

    GrObjectType objType = GET_OBJ_TYPE(grObject);

    if (objType == GR_OBJ_TYPE_IMAGE) {
        GrImage* grImage = (GrImage*)grObject;
        GrDevice* grDevice = GET_OBJ_DEVICE(grObject);

        vkRes = VKD.vkBindImageMemory(grDevice->device, grImage->image,
                                      grGpuMemory->deviceMemory, offset);
    } else if (objType == GR_OBJ_TYPE_DESCRIPTOR_SET ||
               objType == GR_OBJ_TYPE_PIPELINE) {
        // Nothing to do
    } else {
        LOGW("unsupported object type %d\n", objType);
        return GR_ERROR_UNAVAILABLE;
    }

    if (vkRes != VK_SUCCESS) {
        LOGW("binding failed (%d)\n", objType);
    }
    return getGrResult(vkRes);
}
