/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may not use this file except in compliance
 * with the License. A copy of the License is located at
 *
 * http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
 * and limitations under the License.
 */
#include "../build/include/ai_djl_pytorch_jni_PyTorchLibrary.h"
#include "djl_pytorch_jni_error.h"
#include "djl_pytorch_jni_utils.h"

// The file is the implementation for PyTorch tensor core functionality operation

JNIEXPORT jlongArray JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchSizes(
    JNIEnv* env, jobject jthis, jobject jhandle) {
  API_BEGIN();
  const auto* tensor_ptr = utils::GetPointerFromJHandle<torch::Tensor>(env, jhandle);
  jlongArray size = env->NewLongArray(tensor_ptr->dim());
  env->SetLongArrayRegion(size, 0, tensor_ptr->dim(), reinterpret_cast<const jlong*>(tensor_ptr->sizes().data()));
  return size;
  API_END();
}

JNIEXPORT jint JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchDType(JNIEnv* env, jobject jthis, jobject jhandle) {
  API_BEGIN();
  const auto* tensor_ptr = utils::GetPointerFromJHandle<torch::Tensor>(env, jhandle);
  return utils::GetDTypeFromScalarType(tensor_ptr->scalar_type());
  API_END();
}

JNIEXPORT jintArray JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchDevice(
    JNIEnv* env, jobject jthis, jobject jhandle) {
  jclass jexception = env->FindClass("java/lang/NullPointerException");
  API_BEGIN();
  const auto* tensor_ptr = utils::GetPointerFromJHandle<torch::Tensor>(env, jhandle);
  jintArray result = env->NewIntArray(2);
  if (result == nullptr) {
    env->ThrowNew(jexception, "Unable to create int array");
  }
  jint temp_device[] = {static_cast<int>(tensor_ptr->device().type()), tensor_ptr->device().index()};
  env->SetIntArrayRegion(result, 0, 2, temp_device);
  return result;
  API_END();
}

JNIEXPORT jint JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchLayout(JNIEnv* env, jobject jthis, jobject jhandle) {
  API_BEGIN();
  jclass jexception = env->FindClass("java/lang/IllegalStateException");
  const auto* tensor_ptr = utils::GetPointerFromJHandle<torch::Tensor>(env, jhandle);
  auto layout = tensor_ptr->layout();
  switch (layout) {
    case torch::kStrided:
      return 0;
    case torch::kSparse:
      return 1;
    case torch::kMkldnn:
      return 2;
    default:
      env->ThrowNew(jexception, "Internal PyTorch error, layout should only have kStrided, kSparse or kMkldnn");
  }
  API_END();
}

JNIEXPORT jobject JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchTo(
    JNIEnv* env, jobject jthis, jobject jhandle, jint jdtype, jintArray jdevice, jboolean jcopy) {
  API_BEGIN();
  const auto* tensor_ptr = utils::GetPointerFromJHandle<torch::Tensor>(env, jhandle);
  const auto device = utils::GetDeviceFromJDevice(env, jdevice);
  const auto* result_ptr =
      new torch::Tensor(tensor_ptr->to(device, utils::GetScalarTypeFromDType(jdtype), false, jcopy == JNI_TRUE));
  return utils::CreatePointer<torch::Tensor>(env, result_ptr);
  API_END();
}

JNIEXPORT jobject JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_tensorClone(
    JNIEnv* env, jobject jthis, jobject jhandle) {
  API_BEGIN();
  const auto* tensor_ptr = utils::GetPointerFromJHandle<torch::Tensor>(env, jhandle);
  const auto* result_ptr = new torch::Tensor(tensor_ptr->clone());
  return utils::CreatePointer<torch::Tensor>(env, result_ptr);
  API_END();
}

JNIEXPORT jobject JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchSlice(
    JNIEnv* env, jobject jthis, jobject jhandle, jlong jdim, jlong jstart, jlong jend, jlong jstep) {
  API_BEGIN();
  const auto* tensor_ptr = utils::GetPointerFromJHandle<torch::Tensor>(env, jhandle);
  const auto* result_ptr = new torch::Tensor(tensor_ptr->slice(jdim, jstart, jend, jstep));
  return utils::CreatePointer<torch::Tensor>(env, result_ptr);
  API_END();
}

JNIEXPORT jobject JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchMaskedSelect(
    JNIEnv* env, jobject jthis, jobject jhandle, jobject jmasked_handle) {
  API_BEGIN();
  const auto* tensor_ptr = utils::GetPointerFromJHandle<torch::Tensor>(env, jhandle);
  const auto* index_ptr = utils::GetPointerFromJHandle<torch::Tensor>(env, jmasked_handle);
  const auto* result_ptr = new torch::Tensor(tensor_ptr->masked_select(*index_ptr));
  return utils::CreatePointer<torch::Tensor>(env, result_ptr);
  API_END();
}

JNIEXPORT jbyteArray JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchDataPtr(
    JNIEnv* env, jobject jthis, jobject jhandle) {
  API_BEGIN();
  jclass jexception = env->FindClass("java/lang/IllegalStateException");
  const auto* tensor_ptr = utils::GetPointerFromJHandle<torch::Tensor>(env, jhandle);
  // Currently data_ptr() only support contiguous
  if (!tensor_ptr->is_contiguous()) {
    env->ThrowNew(jexception, "Currently data_ptr() only supports contiguous tensors");
  }
  jbyteArray result = env->NewByteArray(tensor_ptr->nbytes());
  env->SetByteArrayRegion(result, 0, tensor_ptr->nbytes(), static_cast<const jbyte*>(tensor_ptr->data_ptr()));
  return result;
  API_END();
}

JNIEXPORT void JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchDeleteTensor(
    JNIEnv* env, jobject jthis, jobject jhandle) {
  const auto* tensor_ptr = utils::GetPointerFromJHandle<const torch::Tensor>(env, jhandle);
  delete tensor_ptr;
}

JNIEXPORT jobject JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchLogicalXor(
    JNIEnv* env, jobject jthis, jobject jself, jobject jother) {
  API_BEGIN();
  const auto* self_ptr = utils::GetPointerFromJHandle<const torch::Tensor>(env, jself);
  const auto* other_ptr = utils::GetPointerFromJHandle<const torch::Tensor>(env, jother);
  const auto* result_ptr = new torch::Tensor(torch::logical_xor(*self_ptr, *other_ptr));
  return utils::CreatePointer<torch::Tensor>(env, result_ptr);
  API_END();
}

JNIEXPORT jobject JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchLogicalNot(
    JNIEnv* env, jobject jthis, jobject jhandle) {
  API_BEGIN();
  const auto* tensor_ptr = utils::GetPointerFromJHandle<const torch::Tensor>(env, jhandle);
  const auto* result_ptr = new torch::Tensor(tensor_ptr->logical_not());
  return utils::CreatePointer<torch::Tensor>(env, result_ptr);
  API_END();
}
