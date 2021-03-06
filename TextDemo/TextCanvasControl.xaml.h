﻿//
// TextCanvasControl.xaml.h
// TextCanvasControl 类的声明
//

#pragma once

#include "TextCanvasControl.g.h"
#include "ILayoutItem.h"

using namespace Windows::UI::Xaml::Media::Imaging;

namespace TextDemo
{
	ref class TextMask;
	ref class MainPage;
	
	ref class TextAttribute;
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class TextCanvasControl sealed
	{
	public:
		TextCanvasControl(MainPage^ page);
		void initTextCanvas(double width,double height,double scale,WriteableBitmap^ previewImg);
		void addTextLayoutItem(ILayoutItem^ item);	
		void removeTextLayoutItem(ILayoutItem^ item);
		
		Platform::Array<TextAttribute^>^ getItemAttributes();
		Platform::Array<BYTE>^ getResultData();
		int getResultWidth();
		int getResultHeight();
		void updateTextMask();

	private:
		

		double				m_scale;
		WriteableBitmap^	m_previewImg;
		TextMask^			m_textMask;
		MainPage^			m_pAdjustPage;
		void OnTextCanvasPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);
	};
}
