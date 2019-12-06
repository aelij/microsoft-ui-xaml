﻿// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "pch.h"
#include "NavigationViewItemsFactory.h"
#include "NavigationViewItemBase.h"
#include "NavigationViewItem.h"
#include "ItemTemplateWrapper.h"


NavigationViewItemsFactory::NavigationViewItemsFactory()
{
}

void NavigationViewItemsFactory::UserElementFactory(winrt::IInspectable newValue)
{
    m_itemTemplateWrapper = newValue.try_as<winrt::IElementFactoryShim>();
    if (!m_itemTemplateWrapper)
    {
        // ItemTemplate set does not implement IElementFactoryShim. We also 
        // want to support DataTemplate and DataTemplateSelectors automagically.
        if (auto dataTemplate = newValue.try_as<winrt::DataTemplate>())
        {
            m_itemTemplateWrapper = winrt::make<ItemTemplateWrapper>(dataTemplate);
        }
        else if (auto selector = newValue.try_as<winrt::DataTemplateSelector>())
        {
            m_itemTemplateWrapper = winrt::make<ItemTemplateWrapper>(selector);
        }
    }
}

winrt::UIElement NavigationViewItemsFactory::GetElementCore(winrt::ElementFactoryGetArgs const& args)
{
    winrt::IInspectable newContent = args.Data();
    // Attempt to get NavigationViewItemBase from user defined ItemTemplate
    if (m_itemTemplateWrapper)
    {
        auto element = m_itemTemplateWrapper.GetElement(args);
        if (element.try_as<winrt::NavigationViewItemBase>())
        {
            return element;
        }

        newContent = element;
    }

    //Check whether the data is its own container
    if (auto data = args.Data().try_as<winrt::NavigationViewItemBase>())
    {
        return data;
    }

    // Create a wrapping container for the data
    auto nvi = winrt::make_self<NavigationViewItem>();
    nvi->Content(newContent);
    return *nvi;
}

void NavigationViewItemsFactory::RecycleElementCore(winrt::ElementFactoryRecycleArgs const& args)
{
    if (m_itemTemplateWrapper)
    {
        m_itemTemplateWrapper.RecycleElement(args);
    }
    else
    {
        // We want to unlink the containers from the parent repeater
        // in case we are required to move it to a different repeater.
        if (auto panel = args.Parent().try_as<winrt::Panel>())
        {
            auto children = panel.Children();
            unsigned int childIndex = 0;
            if (children.IndexOf(args.Element(), childIndex))
            {
                children.RemoveAt(childIndex);
            }
        }
    }
}
