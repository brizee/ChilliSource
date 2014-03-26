//
//  FacebookPostSystem.cpp
//  Chilli Source
//
//  Created by Stuart McGaw on 06/06/2011.
//  Copyright 2011 Tag Games. All rights reserved.
//

#include <ChilliSource/Backend/Platform/iOS/Social/Facebook/FacebookPostSystem.h>

#include <ChilliSource/Core/Base/MakeDelegate.h>
#include <ChilliSource/Social/Facebook/FacebookAuthenticationSystem.h>

#include <FacebookSDK/FacebookSDK.h>

#ifdef __IPHONE_6_0
#include <UIKit/UIKit.h>
#include <Social/Social.h>
#include <ChilliSource/Backend/Platform/iOS/Core/Base/EAGLView.h>
#include <ChilliSource/Backend/Platform/iOS/Social/Facebook/FacebookUtils.h>
#include <ChilliSource/Backend/Platform/iOS/Social/Facebook/FacebookUtils.h>
#include <ChilliSource/Backend/Platform/iOS/Social/Facebook/FacebookUtils.h>
#endif

namespace ChilliSource
{
	namespace iOS
	{
        namespace
        {
            //----------------------------------------------------
            /// Convert the post description to a newly allocated
            /// NSDictionary which is what the iOS framework deals
            /// with
            ///
            /// @author S Downie
            ///
            /// @param Post description
            /// @return Ownership of new dictionary
            //----------------------------------------------------
            NSDictionary* CreateNSDisctionaryFromPostDesc(const Social::FacebookPostSystem::PostDesc& in_desc)
            {
                NSDictionary* postParams = [[NSMutableDictionary alloc] initWithObjectsAndKeys:
                                                                                           Core::StringUtils::StringToNSString(in_desc.m_url), @"link",
                                                                                           Core::StringUtils::StringToNSString(in_desc.m_picUrl), @"picture",
                                                                                           Core::StringUtils::StringToNSString(in_desc.m_name), @"name",
                                                                                           Core::StringUtils::StringToNSString(in_desc.m_caption), @"caption",
                                                                                           Core::StringUtils::StringToNSString(in_desc.m_description), @"description",
                                                                                           Core::StringUtils::StringToNSString(in_desc.m_to), @"to", nil];
                
                return postParams;
            }
        }
        
        CS_DEFINE_NAMEDTYPE(FacebookPostSystem);
        
        //----------------------------------------------------
        //----------------------------------------------------
		FacebookPostSystem::FacebookPostSystem(Social::FacebookAuthenticationSystem* in_authSystem)
        : m_authSystem(in_authSystem)
		{
#if CS_ENABLE_DEBUG
            [FBSettings enableBetaFeature:FBBetaFeaturesShareDialog];
#endif
		}
		//----------------------------------------------------
        //----------------------------------------------------
		bool FacebookPostSystem::IsA(Core::InterfaceIDType in_interfaceID) const
		{
			return FacebookPostSystem::InterfaceID == in_interfaceID || Social::FacebookPostSystem::InterfaceID == in_interfaceID;
		}
        //----------------------------------------------------
        //----------------------------------------------------
		void FacebookPostSystem::Post(const PostDesc& in_desc, const PostResultDelegate& in_delegate)
		{
            CS_ASSERT(m_postCompleteDelegate == nullptr, "Cannot post more than once at a time");
            CS_ASSERT(m_authSystem->IsSignedIn() == true, "User must be authenticated to post");
            CS_ASSERT(m_authSystem->HasPermission("publish_actions") == true, "User must have publish_actions write permission granted");
            CS_ASSERT(in_desc.m_to.empty() == true || m_authSystem->HasPermission("publish_stream") == true, "User must have publish_stream write permissions in order to post to their wall");
            
			m_postCompleteDelegate = in_delegate;
			
            //Currently the iOS native dialog does not provide all the functionality of the web based one so we
            //cannot use it
            bool canPostNatively = false;//([[[UIDevice currentDevice] systemVersion] floatValue] >= 6.0f);
            
            if(canPostNatively)
            {
                PostNative(in_desc);
            }
            else
            {
                PostWeb(in_desc);
            }
        }
        //----------------------------------------------------
        //----------------------------------------------------
        void FacebookPostSystem::SendRequest(const RequestDesc& in_desc, const PostResultDelegate& in_delegate)
        {
            CS_ASSERT(m_requestCompleteDelegate == nullptr, "Cannot request more than once at a time");
            CS_ASSERT(m_authSystem->IsSignedIn() == true, "User must be authenticated to request");
            
            //Construct a list of comma separated IDs
            std::string recipients;
            Core::StringUtils::ToCSV(in_desc.m_recipients, recipients);
            
            m_requestCompleteDelegate = in_delegate;
            
            NSString* requestType = @"to";
            
            switch (in_desc.m_type)
            {
                case RequestRecipientMode::k_fixed:
                    requestType = @"to";
                    break;
                case RequestRecipientMode::k_optional:
                    requestType = @"suggestions";
                    break;
            }
            
            NSMutableDictionary* params = [NSMutableDictionary dictionaryWithObjectsAndKeys:Core::StringUtils::StringToNSString(recipients), requestType, nil];
            
            [FBWebDialogs presentRequestsDialogModallyWithSession:nil
                                                          message:Core::StringUtils::StringToNSString(in_desc.m_description)
                                                            title:Core::StringUtils::StringToNSString(in_desc.m_caption)
                                                       parameters:params
                                                          handler:^(FBWebDialogResult result, NSURL *resultURL, NSError *error)
             {
                 if(!error)
                 {
                     if(m_requestCompleteDelegate)
                     {
                         if(result == FBWebDialogResultDialogCompleted)
                         {
                             NSDictionary *urlParams = [URLParser parseURLParams:[resultURL query]];
                             if (![urlParams valueForKey:@"request"])
                             {
                                 // User clicked the Cancel button
                                m_requestCompleteDelegate(Social::FacebookPostSystem::PostResult::k_cancelled);
                                m_requestCompleteDelegate = nullptr;
                             }
                             else
                             {
                                // User clicked the Share button
                                m_requestCompleteDelegate(Social::FacebookPostSystem::PostResult::k_success);
                                m_requestCompleteDelegate = nullptr;
                             }
                         }
                         else
                         {
                             // User clicked the X button
                             m_requestCompleteDelegate(Social::FacebookPostSystem::PostResult::k_cancelled);
                             m_requestCompleteDelegate = nullptr;
                         }
                     }
                 }
                 else
                 {
                     NSLog(@"%@", error.localizedDescription);
                     NSLog(@"%@", error.description);
             
                     if(m_requestCompleteDelegate)
                     {
                         m_requestCompleteDelegate(Social::FacebookPostSystem::PostResult::k_failed);
                         m_requestCompleteDelegate = nullptr;
                     }
                 }
             }];
        }
        //----------------------------------------------------
        //----------------------------------------------------
        void FacebookPostSystem::PostNative(const PostDesc& in_desc)
        {
            [FBDialogs presentOSIntegratedShareDialogModallyFrom:[UIApplication sharedApplication].keyWindow.rootViewController
                                                     initialText:Core::StringUtils::StringToNSString(in_desc.m_name + " " + in_desc.m_description)
                                                           image:nil
                                                             url:[NSURL URLWithString:Core::StringUtils::StringToNSString(in_desc.m_url)]
                                                         handler:^(FBOSIntegratedShareDialogResult result, NSError *error)
            {
                if (error)
                {
                    NSLog(@"%@", error.localizedDescription);

                    if(m_postCompleteDelegate)
                    {
                        m_postCompleteDelegate(Social::FacebookPostSystem::PostResult::k_failed);
                        m_postCompleteDelegate = nullptr;
                    }
                }
                else if(m_postCompleteDelegate)
                {
                    switch(result)
                    {
                        case FBNativeDialogResultSucceeded:
                            m_postCompleteDelegate(Social::FacebookPostSystem::PostResult::k_success);
                            break;
                        case FBNativeDialogResultCancelled:
                            m_postCompleteDelegate(Social::FacebookPostSystem::PostResult::k_cancelled);
                            break;
                        case FBNativeDialogResultError:
                            m_postCompleteDelegate(Social::FacebookPostSystem::PostResult::k_failed);
                            break;
                    }
             
                    m_postCompleteDelegate = nullptr;
                }
            }];
        }
        //----------------------------------------------------
        //----------------------------------------------------
        void FacebookPostSystem::PostWeb(const PostDesc& in_desc)
        {
            //If the user is posting to nobody then they are posting to
            //their own wall.
            std::string toUser = "me";
            if(!in_desc.m_to.empty())
            {
                toUser = in_desc.m_to;
            }
            
            NSDictionary* postParams = CreateNSDisctionaryFromPostDesc(in_desc);
            
            [FBWebDialogs presentFeedDialogModallyWithSession:nil
                                                   parameters:postParams
                                                      handler:^(FBWebDialogResult result, NSURL *resultURL, NSError *error)
            {
                if(!error)
                {
                    if(m_postCompleteDelegate)
                    {
                        if(result == FBWebDialogResultDialogCompleted)
                        {
                            NSDictionary *urlParams = [URLParser parseURLParams:[resultURL query]];
                            if (![urlParams valueForKey:@"post_id"])
                            {
                                // User clicked the Cancel button
                                m_postCompleteDelegate(Social::FacebookPostSystem::PostResult::k_cancelled);
                                m_postCompleteDelegate = nullptr;
                            }
                            else
                            {
                                // User clicked the Share button
                                m_postCompleteDelegate(Social::FacebookPostSystem::PostResult::k_success);
                                m_postCompleteDelegate = nullptr;
                            }
                        }
                        else
                        {
                            // User clicked the X button
                            m_postCompleteDelegate(Social::FacebookPostSystem::PostResult::k_cancelled);
                            m_postCompleteDelegate = nullptr;
                        }
                    }
                }
                else
                {
                    NSLog(@"%@", error.localizedDescription);
                    NSLog(@"%@", error.description);
             
                    if(m_postCompleteDelegate)
                    {
                        m_postCompleteDelegate(Social::FacebookPostSystem::PostResult::k_failed);
                        m_postCompleteDelegate = nullptr;
                    }
                }
            }];
            
            [postParams release];
        }
	}
}
