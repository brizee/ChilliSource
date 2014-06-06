//
//  KeyboardJavaInterface.cpp
//  Chilli Source
//  Created by Ian Copland on 04/02/2014.
//
//  The MIT License (MIT)
//
//  Copyright (c) 2014 Tag Games Limited
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#ifdef CS_TARGETPLATFORM_ANDROID

#include <CSBackend/Platform/Android/Input/Keyboard/KeyboardJavaInterface.h>

#include <CSBackend/Platform/Android/Core/JNI/JavaInterfaceManager.h>
#include <CSBackend/Platform/Android/Core/JNI/JavaInterfaceUtils.h>
#include <ChilliSource/Core/Base/Application.h>
#include <ChilliSource/Core/Threading/TaskScheduler.h>

#include <jni.h>

//-----------------------------------------------
/// C function declarations
//-----------------------------------------------
extern "C"
{
	void Java_com_chillisource_input_KeyboardNativeInterface_NativeOnTextAdded(JNIEnv* inpEnv, jobject inThis, jstring injstrText);
	void Java_com_chillisource_input_KeyboardNativeInterface_NativeOnTextDeleted(JNIEnv* inpEnv, jobject inThis);
	void Java_com_chillisource_input_KeyboardNativeInterface_NativeOnKeyboardDismissed(JNIEnv* inpEnv, jobject inThis);
}
//-----------------------------------------------
/// Native On Text Added
///
/// Called from java when new keyboard text has
/// been received to be added.
///
/// @param The jni environment.
/// @param The java object calling the function.
/// @param The additional text.
//-----------------------------------------------
void Java_com_chillisource_input_KeyboardNativeInterface_NativeOnTextAdded(JNIEnv* inpEnv, jobject inThis, jstring injstrText)
{
	CSBackend::Android::KeyboardJavaInterfaceSPtr pKeyboardJI = CSBackend::Android::JavaInterfaceManager::GetSingletonPtr()->GetJavaInterface<CSBackend::Android::KeyboardJavaInterface>();
	if (pKeyboardJI != nullptr)
	{
		std::string strText = CSBackend::Android::JavaInterfaceUtils::CreateSTDStringFromJString(injstrText);
		auto task = std::bind(&CSBackend::Android::KeyboardJavaInterface::OnTextAdded, pKeyboardJI.get(), strText);
		CSCore::Application::Get()->GetTaskScheduler()->ScheduleMainThreadTask(task);
	}
	inpEnv->DeleteLocalRef(injstrText);
}
//-----------------------------------------------
/// Native On Text Deleted
///
/// Called from java when a keyboard text deletion
/// event is received.
///
/// @param The jni environment.
/// @param The java object calling the function.
//-----------------------------------------------
void Java_com_chillisource_input_KeyboardNativeInterface_NativeOnTextDeleted(JNIEnv* inpEnv, jobject inThis)
{
	CSBackend::Android::KeyboardJavaInterfaceSPtr pKeyboardJI = CSBackend::Android::JavaInterfaceManager::GetSingletonPtr()->GetJavaInterface<CSBackend::Android::KeyboardJavaInterface>();
	if (pKeyboardJI != nullptr)
	{
		auto task = std::bind(&CSBackend::Android::KeyboardJavaInterface::OnTextDeleted, pKeyboardJI.get());
		CSCore::Application::Get()->GetTaskScheduler()->ScheduleMainThreadTask(task);
	}
}
//-----------------------------------------------
/// Native On Keyboard Dismissed
///
/// Called from java when the keyboard is dismissed.
///
/// @param The jni environment.
/// @param The java object calling the function.
//-----------------------------------------------
void Java_com_chillisource_input_KeyboardNativeInterface_NativeOnKeyboardDismissed(JNIEnv* inpEnv, jobject inThis)
{
	CSBackend::Android::KeyboardJavaInterfaceSPtr pKeyboardJI = CSBackend::Android::JavaInterfaceManager::GetSingletonPtr()->GetJavaInterface<CSBackend::Android::KeyboardJavaInterface>();
	if (pKeyboardJI != nullptr)
	{
		auto task = std::bind(&CSBackend::Android::KeyboardJavaInterface::OnKeyboardDismissed, pKeyboardJI.get());
		CSCore::Application::Get()->GetTaskScheduler()->ScheduleMainThreadTask(task);
	}
}

namespace CSBackend
{
	namespace Android
	{
		namespace
		{
			//-----------------------------------------------
			/// Keyboard Type To Integer
			///
			/// Converts from the keyboard type enum to an
			/// integer that can be passed on to the Java
			/// side of the engine.
			///
			/// @param The keyboard type to convert.
			/// @return The keyboard type in integer form.
			//-----------------------------------------------
			s32 KeyboardTypeToInteger(CSInput::Keyboard::Type ineKeyboardType)
			{
				switch (ineKeyboardType)
				{
				case CSInput::Keyboard::Type::k_text:
					return 0;
				case CSInput::Keyboard::Type::k_numeric:
					return 1;
				default:
					CS_LOG_ERROR("Invalid keyboard type, cannot be converted.");
					return 0;
				}
			}
			//-----------------------------------------------
			/// Keyboard Capitalisation To Integer
			///
			/// Converts from the Keyboard Capitalisation enum
			/// to an integer that can be passed on to the
			/// Java side of the engine.
			///
			/// @param The Keyboard Capitalisation to convert.
			/// @return The Keyboard Capitalisation in integer
			/// form.
			//-----------------------------------------------
			s32 KeyboardCapitalisationToInteger(CSInput::Keyboard::Capitalisation ineKeyboardCapitalisation)
			{
				switch (ineKeyboardCapitalisation)
				{
				case CSInput::Keyboard::Capitalisation::k_none:
					return 0;
				case CSInput::Keyboard::Capitalisation::k_sentences:
					return 1;
				case CSInput::Keyboard::Capitalisation::k_words:
					return 2;
				case CSInput::Keyboard::Capitalisation::k_all:
					return 3;
				default:
					CS_LOG_ERROR("Invalid keyboard capitalisation, cannot be converted.");
					return 0;
				}
			}
		}

		CS_DEFINE_NAMEDTYPE(KeyboardJavaInterface);
		//-----------------------------------------------
		/// Constructor
		//-----------------------------------------------
		KeyboardJavaInterface::KeyboardJavaInterface()
		{
			CreateNativeInterface("com/chillisource/input/KeyboardNativeInterface");
			CreateMethodReference("Activate", "()V");
			CreateMethodReference("Deactivate", "()V");
			CreateMethodReference("SetKeyboardType", "(I)V");
			CreateMethodReference("SetCapitalisationMethod", "(I)V");
		}
		//-----------------------------------------------
		/// Is A
		//-----------------------------------------------
		bool KeyboardJavaInterface::IsA(CSCore::InterfaceIDType inInterfaceID) const
		{
			return (KeyboardJavaInterface::InterfaceID == inInterfaceID);
		}
		//-----------------------------------------------
		/// Set Text Added Delegate
		//-----------------------------------------------
		void KeyboardJavaInterface::SetTextAddedDelegate(const TextAddedDelegate& inDelegate)
		{
			mTextAddedDelegate = inDelegate;
		}
		//-----------------------------------------------
		/// Set Text Deleted Delegate
		//-----------------------------------------------
		void KeyboardJavaInterface::SetTextDeletedDelegate(const TextDeletedDelegate& inDelegate)
		{
			mTextDeletedDelegate = inDelegate;
		}
		//-----------------------------------------------
		/// Set Keyboard Dismissed Delegate
		//-----------------------------------------------
		void KeyboardJavaInterface::SetKeyboardDismissedDelegate(const KeyboardDismissedDelegate& inDelegate)
		{
			mKeyboardDismissedDelegate = inDelegate;
		}
		//-----------------------------------------------
		/// Activate
		//-----------------------------------------------
		void KeyboardJavaInterface::Activate()
		{
			JNIEnv* pEnv = JavaInterfaceManager::GetSingletonPtr()->GetJNIEnvironmentPtr();
			pEnv->CallVoidMethod(GetJavaObject(), GetMethodID("Activate"));
		}
		//-----------------------------------------------
		/// Deactivate
		//-----------------------------------------------
		void KeyboardJavaInterface::Deactivate()
		{
			JNIEnv* pEnv = JavaInterfaceManager::GetSingletonPtr()->GetJNIEnvironmentPtr();
			pEnv->CallVoidMethod(GetJavaObject(), GetMethodID("Deactivate"));
		}
		//-------------------------------------------
		/// Set Keyboard Type
		//-------------------------------------------
        void KeyboardJavaInterface::SetKeyboardType(CSInput::Keyboard::Type ineKeyboardType)
        {
        	JNIEnv* pEnv = JavaInterfaceManager::GetSingletonPtr()->GetJNIEnvironmentPtr();
        	s32 dwKeyboardType = KeyboardTypeToInteger(ineKeyboardType);
        	pEnv->CallVoidMethod(GetJavaObject(), GetMethodID("SetKeyboardType"), dwKeyboardType);
        }
		//-------------------------------------------
		/// Set Capitalisation Method
		//-------------------------------------------
        void KeyboardJavaInterface::SetCapitalisationMethod(CSInput::Keyboard::Capitalisation ineKeyboardCapitalisation)
        {
        	JNIEnv* pEnv = JavaInterfaceManager::GetSingletonPtr()->GetJNIEnvironmentPtr();
        	s32 dwKeyboardCapitalisation = KeyboardCapitalisationToInteger(ineKeyboardCapitalisation);
        	pEnv->CallVoidMethod(GetJavaObject(), GetMethodID("SetCapitalisationMethod"), dwKeyboardCapitalisation);
        }
		//-----------------------------------------------
		/// On Text Added
		//-----------------------------------------------
		void KeyboardJavaInterface::OnTextAdded(const std::string& instrText)
		{
			if (mTextAddedDelegate != nullptr)
			{
				mTextAddedDelegate(instrText);
			}
		}
		//-----------------------------------------------
		/// On Text Deleted
		//-----------------------------------------------
		void KeyboardJavaInterface::OnTextDeleted()
		{
			if (mTextDeletedDelegate != nullptr)
			{
				mTextDeletedDelegate();
			}
		}
		//-----------------------------------------------
		/// On Keyboard Dismissed
		//-----------------------------------------------
		void KeyboardJavaInterface::OnKeyboardDismissed()
		{
			if (mKeyboardDismissedDelegate != nullptr)
			{
				mKeyboardDismissedDelegate();
			}
		}
	}
}

#endif