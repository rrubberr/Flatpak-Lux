/***************************************************************************
 *   Copyright (C) 1998-2009 by authors (see AUTHORS.txt )                 *
 *                                                                         *
 *   This file is part of LuxRender.                                       *
 *                                                                         *
 *   Lux Renderer is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Lux Renderer is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 *   This project is based on PBRT ; see http://www.pbrt.org               *
 *   Lux Renderer website : http://www.luxrender.net                       *
 ***************************************************************************/

#include "ui_tonemap.h"
#include "tonemapwidget.hxx"

#include "mainwindow.hxx"

#include "api.h"

template<class T> inline T Clamp(T val, T low, T high) {
	return val > low ? (val < high ? val : high) : low;
}

ToneMapWidget::ToneMapWidget(QWidget *parent) : QWidget(parent), ui(new Ui::ToneMapWidget)
{
	ui->setupUi(this);
	
	// Tonemap page - only show Reinhard frame by default
	
	ui->frame_toneMapLinear->hide ();
	ui->frame_toneMapContrast->hide ();

	// Reinhard
	connect(ui->comboBox_kernel, SIGNAL(currentIndexChanged(int)), this, SLOT(setTonemapKernel(int)));
	connect(ui->slider_prescale, SIGNAL(valueChanged(int)), this, SLOT(prescaleChanged(int)));
	connect(ui->spinBox_prescale, SIGNAL(valueChanged(double)), this, SLOT(prescaleChanged(double)));
	connect(ui->slider_postscale, SIGNAL(valueChanged(int)), this, SLOT(postscaleChanged(int)));
	connect(ui->spinBox_postscale, SIGNAL(valueChanged(double)), this, SLOT(postscaleChanged(double)));
	connect(ui->slider_burn, SIGNAL(valueChanged(int)), this, SLOT(burnChanged(int)));
	connect(ui->spinBox_burn, SIGNAL(valueChanged(double)), this, SLOT(burnChanged(double)));

	// Linear
	connect(ui->slider_sensitivity, SIGNAL(valueChanged(int)), this, SLOT(sensitivityChanged(int)));
	connect(ui->spinBox_sensitivity, SIGNAL(valueChanged(double)), this, SLOT(sensitivityChanged(double)));
	connect(ui->slider_exposure, SIGNAL(valueChanged(int)), this, SLOT(exposureChanged(int)));
	connect(ui->spinBox_exposure, SIGNAL(valueChanged(double)), this, SLOT(exposureChanged(double)));
	connect(ui->slider_fstop, SIGNAL(valueChanged(int)), this, SLOT(fstopChanged(int)));
	connect(ui->spinBox_fstop, SIGNAL(valueChanged(double)), this, SLOT(fstopChanged(double)));
	connect(ui->slider_gamma_linear, SIGNAL(valueChanged(int)), this, SLOT(gammaLinearChanged(int)));
	connect(ui->spinBox_gamma_linear, SIGNAL(valueChanged(double)), this, SLOT(gammaLinearChanged(double)));
	
	// Max contrast
	connect(ui->slider_ywa, SIGNAL(valueChanged(int)), this, SLOT(ywaChanged(int)));
	connect(ui->spinBox_ywa, SIGNAL(valueChanged(double)), this, SLOT(ywaChanged(double)));
}

ToneMapWidget::~ToneMapWidget()
{
}

void ToneMapWidget::updateWidgetValues()
{
	// Tonemap kernel selection
	setTonemapKernel (m_TM_kernel);

	// Reinhard widgets
	
	updateWidgetValue(ui->slider_prescale, (int)((FLOAT_SLIDER_RES / TM_REINHARD_PRESCALE_RANGE) * m_TM_reinhard_prescale) );
	updateWidgetValue(ui->spinBox_prescale, m_TM_reinhard_prescale);

	updateWidgetValue(ui->slider_postscale, (int)((FLOAT_SLIDER_RES / TM_REINHARD_POSTSCALE_RANGE) * (m_TM_reinhard_postscale)));
	updateWidgetValue(ui->spinBox_postscale, m_TM_reinhard_postscale);

	updateWidgetValue(ui->slider_burn, (int)((FLOAT_SLIDER_RES / TM_REINHARD_BURN_RANGE) * m_TM_reinhard_burn));
	updateWidgetValue(ui->spinBox_burn, m_TM_reinhard_burn);

	// Linear widgets
	updateWidgetValue(ui->slider_exposure, ValueToLogSliderVal(m_TM_linear_exposure, TM_LINEAR_EXPOSURE_LOG_MIN, TM_LINEAR_EXPOSURE_LOG_MAX) );
	updateWidgetValue(ui->spinBox_exposure, m_TM_linear_exposure);

	updateWidgetValue(ui->slider_sensitivity, (int)((FLOAT_SLIDER_RES / TM_LINEAR_SENSITIVITY_RANGE) * m_TM_linear_sensitivity) );
	updateWidgetValue(ui->spinBox_sensitivity, m_TM_linear_sensitivity);

	updateWidgetValue(ui->slider_fstop, (int)((FLOAT_SLIDER_RES / TM_LINEAR_FSTOP_RANGE) * m_TM_linear_fstop));
	updateWidgetValue(ui->spinBox_fstop, m_TM_linear_fstop);

	updateWidgetValue(ui->slider_gamma_linear, (int)((FLOAT_SLIDER_RES / TM_LINEAR_GAMMA_RANGE) * m_TM_linear_gamma));
	updateWidgetValue(ui->spinBox_gamma_linear, m_TM_linear_gamma);

	// Contrast widgets
	updateWidgetValue(ui->slider_ywa, ValueToLogSliderVal(m_TM_contrast_ywa, TM_CONTRAST_YWA_LOG_MIN, TM_CONTRAST_YWA_LOG_MAX) );
	updateWidgetValue(ui->spinBox_ywa, m_TM_contrast_ywa);
}

void ToneMapWidget::resetValues()
{
	m_TM_kernel = 0;

	m_TM_reinhard_prescale = 1.0;
	m_TM_reinhard_postscale = 1.0;
	m_TM_reinhard_burn = 6.0;

	m_TM_linear_exposure = 1.0f;
	m_TM_linear_sensitivity = 50.0f;
	m_TM_linear_fstop = 2.8;
	m_TM_linear_gamma = 1.0;

	m_TM_contrast_ywa = 0.1;
}

void ToneMapWidget::resetFromFilm (bool useDefaults)
{
	m_TM_kernel = (int)retrieveParam( useDefaults, LUX_FILM, LUX_FILM_TM_TONEMAPKERNEL);

	m_TM_reinhard_prescale = retrieveParam( useDefaults, LUX_FILM, LUX_FILM_TM_REINHARD_PRESCALE);
	m_TM_reinhard_postscale = retrieveParam( useDefaults, LUX_FILM, LUX_FILM_TM_REINHARD_POSTSCALE);
	m_TM_reinhard_burn = retrieveParam( useDefaults, LUX_FILM, LUX_FILM_TM_REINHARD_BURN);

	m_TM_linear_exposure = retrieveParam( useDefaults, LUX_FILM, LUX_FILM_TM_LINEAR_EXPOSURE);
	m_TM_linear_sensitivity = retrieveParam( useDefaults, LUX_FILM, LUX_FILM_TM_LINEAR_SENSITIVITY);
	m_TM_linear_fstop = retrieveParam( useDefaults, LUX_FILM, LUX_FILM_TM_LINEAR_FSTOP);
	m_TM_linear_gamma = retrieveParam( useDefaults, LUX_FILM, LUX_FILM_TM_LINEAR_GAMMA);

	m_TM_contrast_ywa = retrieveParam( useDefaults, LUX_FILM, LUX_FILM_TM_CONTRAST_YWA);

	luxSetParameterValue(LUX_FILM, LUX_FILM_TM_REINHARD_PRESCALE, m_TM_reinhard_prescale);
	luxSetParameterValue(LUX_FILM, LUX_FILM_TM_REINHARD_POSTSCALE, m_TM_reinhard_postscale);
	luxSetParameterValue(LUX_FILM, LUX_FILM_TM_REINHARD_BURN, m_TM_reinhard_burn);

	luxSetParameterValue(LUX_FILM, LUX_FILM_TM_LINEAR_EXPOSURE, m_TM_linear_exposure);
	luxSetParameterValue(LUX_FILM, LUX_FILM_TM_LINEAR_SENSITIVITY, m_TM_linear_sensitivity);
	luxSetParameterValue(LUX_FILM, LUX_FILM_TM_LINEAR_FSTOP, m_TM_linear_fstop);
	luxSetParameterValue(LUX_FILM, LUX_FILM_TM_LINEAR_GAMMA, m_TM_linear_gamma);

	luxSetParameterValue(LUX_FILM, LUX_FILM_TM_CONTRAST_YWA, m_TM_contrast_ywa);
}

void ToneMapWidget::setTonemapKernel(int choice)
{
	ui->comboBox_kernel->setCurrentIndex(choice);
	updateParam(LUX_FILM, LUX_FILM_TM_TONEMAPKERNEL, (double)choice);
	m_TM_kernel = choice;
	switch (choice) {
		case 0:
			// Reinhard
			ui->frame_toneMapReinhard->show();
			ui->frame_toneMapLinear->hide();
			ui->frame_toneMapContrast->hide();
			break;
		case 1:
			// Linear
			ui->frame_toneMapReinhard->hide();
			ui->frame_toneMapLinear->show();
			ui->frame_toneMapContrast->hide();
			break;
		case 2:
			// Contrast
			ui->frame_toneMapReinhard->hide();
			ui->frame_toneMapLinear->hide();
			ui->frame_toneMapContrast->show();
			break;
		case 3:
			// MaxWhite
			ui->frame_toneMapReinhard->hide();
			ui->frame_toneMapLinear->hide();
			ui->frame_toneMapContrast->hide();
			break;
		default:
			break;
	}

	emit valuesChanged();
}

void ToneMapWidget::prescaleChanged (int value)
{
	prescaleChanged ((double)value / ( FLOAT_SLIDER_RES / TM_REINHARD_PRESCALE_RANGE ));
}

void ToneMapWidget::prescaleChanged (double value)
{
	m_TM_reinhard_prescale = value;
	int sliderval = (int)((FLOAT_SLIDER_RES / TM_REINHARD_PRESCALE_RANGE) * m_TM_reinhard_prescale);

	updateWidgetValue(ui->slider_prescale, sliderval);
	updateWidgetValue(ui->spinBox_prescale, m_TM_reinhard_prescale);

	updateParam (LUX_FILM, LUX_FILM_TM_REINHARD_PRESCALE, m_TM_reinhard_prescale);

	emit valuesChanged();
}

void ToneMapWidget::postscaleChanged (int value)
{
	postscaleChanged ((double)value / ( FLOAT_SLIDER_RES / TM_REINHARD_PRESCALE_RANGE ));
}

void ToneMapWidget::postscaleChanged (double value)
{
	m_TM_reinhard_postscale = value;
	int sliderval = (int)((FLOAT_SLIDER_RES / TM_REINHARD_POSTSCALE_RANGE) * m_TM_reinhard_postscale);

	updateWidgetValue(ui->slider_postscale, sliderval);
	updateWidgetValue(ui->spinBox_postscale, m_TM_reinhard_postscale);

	updateParam (LUX_FILM, LUX_FILM_TM_REINHARD_POSTSCALE, m_TM_reinhard_postscale);

	emit valuesChanged();
}

void ToneMapWidget::burnChanged (int value)
{
	burnChanged ((double)value / ( FLOAT_SLIDER_RES / TM_REINHARD_BURN_RANGE ) );
}

void ToneMapWidget::burnChanged (double value)
{
	m_TM_reinhard_burn = value;

	int sliderval = (int)((FLOAT_SLIDER_RES / TM_REINHARD_BURN_RANGE) * m_TM_reinhard_burn);

	updateWidgetValue(ui->slider_burn, sliderval);
	updateWidgetValue(ui->spinBox_burn, m_TM_reinhard_burn);

	updateParam(LUX_FILM, LUX_FILM_TM_REINHARD_BURN, m_TM_reinhard_burn);

	emit valuesChanged();
}

void ToneMapWidget::sensitivityChanged (int value)
{
	sensitivityChanged ((double)value / ( FLOAT_SLIDER_RES / TM_LINEAR_SENSITIVITY_RANGE ) );
}

void ToneMapWidget::sensitivityChanged (double value)
{
	m_TM_linear_sensitivity = value;

	int sliderval = (int)((FLOAT_SLIDER_RES / TM_LINEAR_SENSITIVITY_RANGE) * m_TM_linear_sensitivity);

	updateWidgetValue(ui->slider_sensitivity, sliderval);
	updateWidgetValue(ui->spinBox_sensitivity, m_TM_linear_sensitivity);

	updateParam (LUX_FILM, LUX_FILM_TM_LINEAR_SENSITIVITY, m_TM_linear_sensitivity);

	emit valuesChanged();
}

void ToneMapWidget::exposureChanged (int value)
{
	exposureChanged ( LogSliderValToValue(value, TM_LINEAR_EXPOSURE_LOG_MIN, TM_LINEAR_EXPOSURE_LOG_MAX) );
}

void ToneMapWidget::exposureChanged (double value)
{
	m_TM_linear_exposure = value;

	int sliderval = ValueToLogSliderVal (m_TM_linear_exposure, TM_LINEAR_EXPOSURE_LOG_MIN, TM_LINEAR_EXPOSURE_LOG_MAX);

	updateWidgetValue(ui->slider_exposure, sliderval);
	updateWidgetValue(ui->spinBox_exposure, m_TM_linear_exposure);

	updateParam (LUX_FILM, LUX_FILM_TM_LINEAR_EXPOSURE, m_TM_linear_exposure);

	emit valuesChanged();
}

void ToneMapWidget::fstopChanged (int value)
{
	fstopChanged ( (double)value / ( FLOAT_SLIDER_RES / TM_LINEAR_FSTOP_RANGE ) );
}

void ToneMapWidget::fstopChanged (double value)
{
	m_TM_linear_fstop = value;

	int sliderval = (int)((FLOAT_SLIDER_RES / TM_LINEAR_FSTOP_RANGE) * m_TM_linear_fstop);

	updateWidgetValue(ui->slider_fstop, sliderval);
	updateWidgetValue(ui->spinBox_fstop, m_TM_linear_fstop);

	updateParam (LUX_FILM, LUX_FILM_TM_LINEAR_FSTOP, m_TM_linear_fstop);

	emit valuesChanged();
}

void ToneMapWidget::gammaLinearChanged (int value)
{
	gammaLinearChanged ( (double)value / ( FLOAT_SLIDER_RES / TM_LINEAR_GAMMA_RANGE ) );
}

void ToneMapWidget::gammaLinearChanged (double value)
{
	m_TM_linear_gamma = value;

	int sliderval = (int)((FLOAT_SLIDER_RES / TM_LINEAR_GAMMA_RANGE) * m_TM_linear_gamma);

	updateWidgetValue(ui->slider_gamma_linear, sliderval);
	updateWidgetValue(ui->spinBox_gamma_linear, m_TM_linear_gamma);

	updateParam (LUX_FILM, LUX_FILM_TM_LINEAR_GAMMA, m_TM_linear_gamma);

	emit valuesChanged();
}

void ToneMapWidget::ywaChanged (int value)
{
	ywaChanged (LogSliderValToValue(value, TM_CONTRAST_YWA_LOG_MIN, TM_CONTRAST_YWA_LOG_MAX) );
}

void ToneMapWidget::ywaChanged (double value)
{
	m_TM_contrast_ywa = value;

	int sliderval = ValueToLogSliderVal (m_TM_contrast_ywa, TM_CONTRAST_YWA_LOG_MIN, TM_CONTRAST_YWA_LOG_MAX);

	updateWidgetValue(ui->slider_ywa, sliderval);
	updateWidgetValue(ui->spinBox_ywa, m_TM_contrast_ywa);

	updateParam (LUX_FILM, LUX_FILM_TM_CONTRAST_YWA, m_TM_contrast_ywa);

	emit valuesChanged();
}