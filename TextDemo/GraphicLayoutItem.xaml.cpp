﻿//
// GraphicLayoutItem.xaml.cpp
// GraphicLayoutItem 类的实现
//

#include "pch.h"
#include "GraphicLayoutItem.xaml.h"
#include "TextControl.xaml.h"
#include "TextCanvasControl.xaml.h"

using namespace TextDemo;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Devices::Input;
using namespace Windows::Storage;
using namespace Windows::Storage::FileProperties;

using namespace Concurrency;

// “用户控件”项模板在 http://go.microsoft.com/fwlink/?LinkId=234236 上提供

GraphicLayoutItem::GraphicLayoutItem(TextControl^ textControl,TextCanvasControl^ canvasControl)
{
	InitializeComponent();
	m_pTextControl = textControl;
	m_pTextCanvasControl = canvasControl;
	m_textAttribute = m_pTextControl->createTextAttribute();
	m_itemAction = ItemAction::NONE;
	eventDwon_x = 0.0;
	eventDwon_y = 0.0;
	new_x = 0.0;
	new_y = 0.0;
	oldScale = 1.0;
	oldDistance = 1.0;
	oldAngle = 1.0;
	m_isChanged = false;
	m_isPressed = false;
	m_isInit = true;
}

TextAttribute^ GraphicLayoutItem::getTextAttribute()
{
	return m_textAttribute;
}

void GraphicLayoutItem::showSelectBorder(bool isShow)
{
	if(isShow)
	{
		selectBorder->Opacity = 1.0;
		img_delete->Opacity = 1.0;
		img_rotate->Opacity = 1.0;
	}
	else
	{
		selectBorder->Opacity = 0.0;
		img_delete->Opacity = 0.0;
		img_rotate->Opacity = 0.0;
	}
}

void GraphicLayoutItem::notifyChanged()
{
	auto path = L"Assets\\" + m_textAttribute->bgimageName;
	auto packageFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;
	create_task(packageFolder->GetFileAsync(path)).then([this](StorageFile^ file){
		return file->Properties->GetImagePropertiesAsync();
	}).then([this](ImageProperties^ prof){
		auto callback = ref new Windows::UI::Core:: DispatchedHandler([this,prof]()
		{
			auto parent = safe_cast<Canvas^>(this->Parent);	
			double preLeft = parent->GetLeft(this);
			double preTop = parent->GetTop(this);

			selectGrid->Width = prof->Width / 3.0;
			selectGrid->Height = prof->Height / 3.0;

			double preSelectWidth = m_textAttribute->width;
			double preSelectHeight = m_textAttribute->height;
			m_textAttribute->width=selectGrid->Width;
			m_textAttribute->height=selectGrid->Height;



			if(m_isInit)
			{
				moveSelf((parent->ActualWidth - selectGrid->Width)/2.0,(parent->ActualHeight - selectGrid->Height)/2.0);

				m_isInit = false;
			}
			else
			{
				double newSelectWidth = m_textAttribute->width;
				double newSelectHeight = m_textAttribute->height;

				double maxTxtScaleX = parent->ActualWidth / (newSelectWidth + img_delete->Width);
				double maxTxtScaleY = parent->ActualHeight / (newSelectHeight + img_delete->Height);
				double maxTxtScale = maxTxtScaleX < maxTxtScaleY ? maxTxtScaleX : maxTxtScaleY;	

				double preScale = m_textAttribute->scale;
				if(m_textAttribute->scale > maxTxtScale)
					m_textAttribute->scale = maxTxtScale;		
				selectGrid->Width = newSelectWidth * m_textAttribute->scale;
				selectGrid->Height = newSelectHeight * m_textAttribute->scale;		


				//重新调整旋转中心点和位移，保证选择框最终左上点保持不变
				RotateTransform^ preRotateTrans = safe_cast<RotateTransform^>(selectGrid->RenderTransform);
				RotateTransform^ curRotateTrans = ref new RotateTransform();
				curRotateTrans->CenterX = selectGrid->Width / 2.0;
				curRotateTrans->CenterY = selectGrid->Height / 2.0;
				curRotateTrans->Angle = m_textAttribute->angle;

				auto preLeftTop = preRotateTrans->TransformPoint(Point(preLeft,preTop));
				auto curLeftTop = curRotateTrans->TransformPoint(Point(preLeft,preTop));

				parent->SetLeft(this,preLeft + (preLeftTop.X - curLeftTop.X));
				parent->SetTop(this,preTop + (preLeftTop.Y - curLeftTop.Y));

				UpdateLayout();

				rotateSelf(m_textAttribute->angle,curRotateTrans->CenterX,curRotateTrans->CenterY);	

				double newCenterX = parent->GetLeft(this) + curRotateTrans->CenterX;
				double newCenterY = parent->GetTop(this) + curRotateTrans->CenterY;

				m_textAttribute->left = newCenterX - m_textAttribute->width  / 2.0;
				m_textAttribute->top = newCenterY - m_textAttribute->height / 2.0;

			}

			m_pTextCanvasControl->updateTextMask();
			m_pTextControl->EventLock = true;
			m_pTextControl->setCurrentItem(this);
			m_pTextControl->EventLock = false;
		});
		Dispatcher->RunAsync(Windows::UI::Core:: CoreDispatcherPriority ::Normal, callback);
	});	
}

void GraphicLayoutItem::scaleSelf(double scaleValue)
{
	double orgWidth = selectGrid->Width;
	double orgHeight = selectGrid->Height;
	selectGrid->Width = (m_textAttribute->width) * scaleValue;
	selectGrid->Height = (m_textAttribute->height) * scaleValue;
	double disX = (orgWidth - selectGrid->Width) / 2.0;
	double disY = (orgHeight - selectGrid->Height) / 2.0;

	auto parent = safe_cast<Canvas^>(this->Parent);
		
	parent->SetLeft(this,parent->GetLeft(this)+disX);
	parent->SetTop(this,parent->GetTop(this)+disY);
	
}

void GraphicLayoutItem::rotateSelf(double angle,double centerX,double centerY)
{
	
	RotateTransform^ rotateTrans = safe_cast<RotateTransform^>(selectGrid->RenderTransform);
	rotateTrans->Angle = m_textAttribute->angle;
	rotateTrans->CenterX=centerX;
	rotateTrans->CenterY=centerY;
	selectGrid->RenderTransform=rotateTrans;
}

void GraphicLayoutItem::moveSelf(double disX,double disY)
{
	auto parent = safe_cast<Canvas^>(this->Parent);
	double halfW = selectGrid->Width / 2.0;
	double halfH = selectGrid->Height / 2.0;
	double newX = parent->GetLeft(this) + disX;
	double newY = parent->GetTop(this) + disY;	

	if(disX != 0 || disY != 0)
		m_isChanged = true;
	parent->SetLeft(this,parent->GetLeft(this)+disX);
	parent->SetTop(this,parent->GetTop(this)+disY);

	if(m_isInit)
	{
		m_textAttribute->left = newX;
		m_textAttribute->top = newY;
	}
	else
	{
		m_textAttribute->left += disX;
		m_textAttribute->top += disY;
	}


}


void TextDemo::GraphicLayoutItem::DeletePressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_pTextControl->deleteTextItem(this);
	//m_pFunctionControl->saveTextData();
}


void TextDemo::GraphicLayoutItem::DeleteTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e)
{
	e->Handled = true;
}


void TextDemo::GraphicLayoutItem::SelectGridPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	Canvas^ canvas=safe_cast<Canvas^>(this->Parent);
	bringToFront(canvas,this);
	if(sender->GetType()->ToString()->Equals(selectGrid->GetType()->ToString())){
		m_itemAction=MOVE;
		eventDwon_y = e->GetCurrentPoint(canvas)->Position.Y;
		eventDwon_x = e->GetCurrentPoint(canvas)->Position.X;
	}else if(sender->GetType()->ToString()->Equals(img_rotate->GetType()->ToString())){
		m_itemAction=SCALE;
		eventDwon_y = e->GetCurrentPoint(canvas)->Position.Y;
		eventDwon_x=e->GetCurrentPoint(canvas)->Position.X;
		new_x = eventDwon_x;
		new_y = eventDwon_y;			
	}
	e->Handled=true;
	selectGrid->CapturePointer(e->Pointer);
	oldScale=m_textAttribute->scale;
	oldAngle = 0;
	m_isPressed = true;
	m_pTextControl->EventLock = true;
	m_pTextControl->setCurrentItem(this);
	m_pTextControl->EventLock = false;
}


void TextDemo::GraphicLayoutItem::SelectGridReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_itemAction=NONE;
	//拦截事件，禁止再传递
	e->Handled=true;
	selectGrid->ReleasePointerCapture(e->Pointer);
	m_isPressed = false;
	/*if(m_isChanged)
	{
		m_isChanged = false;
		m_pFunctionControl->saveTextData();
		m_pFunctionControl->getAdjustPage()->setNeedSave(true);
	}*/

}


void TextDemo::GraphicLayoutItem::SelectGridMoved(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	Canvas^ canvas=safe_cast<Canvas^>(this->Parent);
	if(m_itemAction==MOVE && e->Pointer->PointerDeviceType != PointerDeviceType::Touch){
		e->Handled=true;
		double disX,disY;
		disX=e->GetCurrentPoint(canvas)->Position.X-eventDwon_x;
		disY=e->GetCurrentPoint(canvas)->Position.Y-eventDwon_y;	
		moveSelf(disX,disY);	
		m_pTextCanvasControl->updateTextMask();
		eventDwon_y = e->GetCurrentPoint(canvas)->Position.Y;
		eventDwon_x = e->GetCurrentPoint(canvas)->Position.X;	
	}else if(m_itemAction==SCALE){
		double x =e->GetCurrentPoint(canvas)->Position.X;
		double y =e->GetCurrentPoint(canvas)->Position.Y;
		double centerX=0;
		double centerY=0;
		centerX=canvas->GetLeft(this) + selectGrid->Width / 2.0;
		centerY=canvas->GetTop(this) + selectGrid->Height / 2.0;
		//起始触摸点和图片中心点的距离
		oldDistance = getDistance(centerX,centerY,eventDwon_x,eventDwon_y);      
		//当前触摸点和图片中心点的距离
		double newDistance = getDistance(centerX,centerY,x,y);
		//得到缩放比例
		double scaleValue = newDistance/oldDistance;

		double newScale=oldScale*scaleValue;

		double orgSelectWidth = m_textAttribute->width;
		double orgSelectHeight = m_textAttribute->height;
		
		double maxTxtScaleX = canvas->ActualWidth / (orgSelectWidth + img_delete->Width);
		double maxTxtScaleY = canvas->ActualHeight / (orgSelectHeight + img_delete->Height);
		double maxTxtScale = maxTxtScaleX < maxTxtScaleY ? maxTxtScaleX : maxTxtScaleY;
		if(newScale > maxTxtScale)
			newScale = maxTxtScale;

		double newWidth = orgSelectWidth * newScale;
		double newHeight = orgSelectHeight * newScale;		

		if(newWidth >= 50 && newHeight >= 50 ){
			if(m_textAttribute->scale != newScale)
				m_isChanged = true;
			//记录到属性
			m_textAttribute->scale=newScale;
			//缩放文本框		
			scaleSelf(m_textAttribute->scale);
		}
		
		//计算旋转角度,并记录到属性
		double angle =  getAngle(centerX,centerY,eventDwon_x,eventDwon_y,x,y) ;		
		double angleOffset = angle - oldAngle;		
		oldAngle = angle;
		if(angleOffset != 0)
			m_isChanged = true;
		m_textAttribute->angle += angleOffset ;
		if(m_textAttribute->angle >= 360) m_textAttribute->angle -= 360;		
		if(m_textAttribute->angle<0) m_textAttribute->angle+=360;
		double gridCenterX = selectGrid->Width / 2.0;
		double gridCenterY = selectGrid->Height / 2.0;
		rotateSelf(m_textAttribute->angle,gridCenterX,gridCenterY);

		////刷新界面控件数据
		m_pTextControl->EventLock = true;
		m_pTextControl->setCurrentItem(this);
		m_pTextControl->EventLock = false;
		m_pTextCanvasControl->updateTextMask();
	}
}


void TextDemo::GraphicLayoutItem::UserControl_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	UpdateLayout();
	notifyChanged();
}
