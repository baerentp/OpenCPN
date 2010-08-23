/******************************************************************************
 * $Id: dial.cpp, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  DashBoard Plugin
 * Author:   Jean-Eudes Onfray
 *           (Inspired by original work from Andreas Heiming)
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#include "dial.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/timer.h>
#include <wx/image.h>
#include <wx/dcbuffer.h>
#include <wx/colordlg.h>
#include <wx/artprov.h>

#include "../../../include/ocpn_plugin.h"

double rad2deg(double angle)
{
      return angle*180.0/M_PI;
}
double deg2rad(double angle)
{
      return angle/180.0*M_PI;
}

DashboardInstrument_Dial::DashboardInstrument_Dial( wxWindow *parent, wxWindowID id, wxString title,
                  int s_angle,
                  int r_angle,
                  int s_value,
                  int e_value) : wxPanel(parent, id)
{
      m_title = title;
      m_AngleStart = s_angle;
      m_AngleRange = r_angle;
      m_MainValueMin = s_value;
      m_MainValueMax = e_value;

      m_MainValue = s_value;
      m_ExtraValue = 0;
      m_MainValueFormat = _T("%d");
      m_MainValueOption = DIAL_POSITION_NONE;
      m_ExtraValueFormat = _T("%d");
      m_ExtraValueOption = DIAL_POSITION_NONE;
      m_MarkerOption = DIAL_MARKER_SIMPLE;
      m_MarkerStep = 1;
      m_LabelStep = 1;
      m_MarkerOffset = 1;
      m_LabelOption = DIAL_LABEL_HORIZONTAL;

      Connect(wxID_ANY, wxEVT_PAINT, wxPaintEventHandler(DashboardInstrument_Dial::OnPaint));
      SetMinSize(wxSize(200, 200));
}

void DashboardInstrument_Dial::SetMainValue(double value)
{
      if (value < m_MainValueMin) m_MainValue = m_MainValueMin;
      else if (value > m_MainValueMax) m_MainValue = m_MainValueMax;
      else m_MainValue = value;

      Refresh(false);
}

void DashboardInstrument_Dial::SetExtraValue(double value)
{
      m_ExtraValue = value;

      Refresh(false);
}

void DashboardInstrument_Dial::OnPaint(wxPaintEvent &WXUNUSED(event))
{
      wxPaintDC dc(this);

      wxRect rect = GetClientRect();

      if(rect.width == 0 || rect.height == 0)
      {
            return;
      }

      wxBitmap bmp;

      // Create a double buffer to draw the plot
      // on screen to prevent flicker from occuring.
      wxBufferedDC buff_dc;
      buff_dc.Init(&dc, bmp);
      buff_dc.Clear();

      m_cx = rect.width / 2;
      m_cy = rect.height / 2 + 10;
      //m_radius = ((rect.height/2)*5)/6;
      m_radius = rect.height*.4;

      DrawFrame(&buff_dc);
      DrawMarkers(&buff_dc);
      DrawLabels(&buff_dc);
      DrawBackground(&buff_dc);
      DrawData(&buff_dc, m_MainValue, m_MainValueFormat, m_MainValueOption);
      DrawData(&buff_dc, m_ExtraValue, m_ExtraValueFormat, m_ExtraValueOption);
      DrawForeground(&buff_dc);
}

void DashboardInstrument_Dial::DrawFrame(wxDC* dc)
{
      wxColour cl;

      GetGlobalColor(_T("UIBDR"), &cl);
      dc->SetBackground(cl);
      dc->Clear();

      dc->SetFont(*OCPNGetFont(_T("DashBoard Label"), 9));
//      dc->SetTextForeground(pFontMgr->GetFontColor(_T("DashBoard Label")));
      GetGlobalColor(_T("BLUE2"), &cl);
      dc->SetTextForeground(cl);
      dc->DrawText(m_title, 5, 5);

      dc->SetBrush(*wxTRANSPARENT_BRUSH);

      wxPen pen;
      pen.SetStyle(wxSOLID);
      GetGlobalColor(_T("BLUE2"), &cl);
      pen.SetColour(cl);
      dc->SetPen(pen);

      dc->DrawCircle(m_cx, m_cy, m_radius);
}

void DashboardInstrument_Dial::DrawMarkers(wxDC* dc)
{
      if (m_MarkerOption == DIAL_MARKER_NONE)
            return;

      wxColour cl;
      GetGlobalColor(_T("BLUE2"), &cl);
      wxPen pen;
      pen.SetStyle(wxSOLID);
      pen.SetColour(cl);
      dc->SetPen(pen);

      int diff_angle = m_AngleStart + m_AngleRange - ANGLE_OFFSET;
      // angle between markers
      double abm = m_AngleRange * m_MarkerStep / (m_MainValueMax - m_MainValueMin);
      // don't draw last value, it's already done as first
      if (m_AngleRange == 360) diff_angle -= abm;

      int offset = 0;
      for(double angle = m_AngleStart - ANGLE_OFFSET; angle <= diff_angle; angle += abm)
      {
            if (m_MarkerOption == DIAL_MARKER_REDGREEN)
            {
                  int a = int(angle + ANGLE_OFFSET) % 360;
                  if (a > 180)
                        cl = *wxRED;
                  else if ((a > 0) && (a < 180))
                        cl = *wxGREEN;
                  else
                        GetGlobalColor(_T("BLUE2"), &cl);

                  pen.SetColour(cl);
                  dc->SetPen(pen);
            }

            double size = 0.92;
            if(offset % m_MarkerOffset)
            {
                  size = 0.96;
            }
            offset++;

            dc->DrawLine(m_cx + (m_radius * size * cos(deg2rad(angle))),
                        m_cy + (m_radius * size * sin(deg2rad(angle))),
                        m_cx + (m_radius * cos(deg2rad(angle))),
                        m_cy + (m_radius * sin(deg2rad(angle))));
      }
      // We must reset pen color so following drawings are fine
      if (m_MarkerOption == DIAL_MARKER_REDGREEN)
      {
            GetGlobalColor(_T("BLUE2"), &cl);
            pen.SetStyle(wxSOLID);
            pen.SetColour(cl);
            dc->SetPen(pen);
      }
}

void DashboardInstrument_Dial::DrawLabels(wxDC* dc)
{
      if (m_LabelOption == DIAL_LABEL_NONE)
            return;

      wxPoint TextPoint;
      wxPen pen;

      wxFont font;
      font.SetFamily(wxFONTFAMILY_ROMAN);
      font.SetPointSize(8);
      dc->SetFont(font);

      wxColor cl;
      GetGlobalColor(_T("BLUE2"), &cl);
      dc->SetTextForeground(cl);

      int diff_angle = m_AngleStart + m_AngleRange - ANGLE_OFFSET;
      // angle between markers
      double abm = m_AngleRange * m_LabelStep / (m_MainValueMax - m_MainValueMin);
      // don't draw last value, it's already done as first
      if (m_AngleRange == 360) diff_angle -= abm;

      int offset = 0;
      int value = m_MainValueMin;
      int width, height;
      for(double angle = m_AngleStart - ANGLE_OFFSET; angle <= diff_angle; angle += abm)
      {
            wxString label = (m_LabelArray.GetCount() ? m_LabelArray.Item(offset) : wxString::Format(_T("%d"), value));
            dc->GetTextExtent(label.c_str(), &width, &height, 0, 0, &font);

            double halfW = width / 2;
            if (m_LabelOption == DIAL_LABEL_HORIZONTAL)
            {
                  double halfH = height / 2;
                  //double delta = sqrt(width*width+height*height);
                  double delta = sqrt(halfW*halfW+halfH*halfH);
                  TextPoint.x = m_cx + ((m_radius * 0.90) - delta) * cos(deg2rad(angle)) - halfW;
                  TextPoint.y = m_cy + ((m_radius * 0.90) - delta) * sin(deg2rad(angle)) - halfH;
                  dc->DrawText(label, TextPoint);
            }
            else if (m_LabelOption == DIAL_LABEL_ROTATED)
            {
                  // The coordinates of dc->DrawRotatedText refer to the top-left corner
                  // of the rectangle bounding the string. So we must calculate the
                  // right coordinates depending of the angle.
                  // Move left from the Marker so that the position is in the Middle of Text
                  long double tmpangle = angle - rad2deg(asin(halfW / (0.90 * m_radius)));
                  TextPoint.x = m_cx + m_radius * 0.90 * cos(deg2rad(tmpangle));
                  TextPoint.y = m_cy + m_radius * 0.90 * sin(deg2rad(tmpangle));

                  dc->DrawRotatedText(label, TextPoint, -90 - angle);
            }
            offset++;
            value += m_LabelStep;
      }
}

void DashboardInstrument_Dial::DrawBackground(wxDC* dc)
{
      // Nothing to do here right now, will be overwritten
      // by child classes if required
}

void DashboardInstrument_Dial::DrawData(wxDC* dc, double value,
            wxString format, DialPositionOption position)
{
      if (position == DIAL_POSITION_NONE)
            return;

      wxFont *font = OCPNGetFont(_T("DashBoard Label"), 9);
      dc->SetFont(*font);
//      dc->SetTextForeground(pFontMgr->GetFontColor(_T("DashBoard Label")));
      wxColour cl;
      GetGlobalColor(_T("BLUE1"), &cl);
      dc->SetTextForeground(cl);

      wxRect rect = GetClientRect();
      wxString text = wxString::Format(format, value);
      int width, height;
      dc->GetTextExtent(text, &width, &height, 0, 0, font);
      wxPoint TextPoint;
      switch (position)
      {
            case DIAL_POSITION_NONE:
                  // This case was already handled before, it's here just
                  // to avoid compiler warning.
                  return;
            case DIAL_POSITION_INSIDE:
                  TextPoint.x = m_cx - (width / 2);
                  TextPoint.y = (rect.height * .75) - height;
                  GetGlobalColor(_T("UIBDR"), &cl);
                  dc->SetBrush(cl);
                  // There might be a background drawn below
                  // so we must clear it first.
                  dc->DrawRectangle(TextPoint.x-2, TextPoint.y-2, width+4, height+4);
                  break;
            case DIAL_POSITION_TOPLEFT:
                  TextPoint.x = 5;
                  TextPoint.y = 20;
                  break;
            case DIAL_POSITION_TOPRIGHT:
                  TextPoint.x = rect.width-width-5;
                  TextPoint.y = 20;
                  break;
            case DIAL_POSITION_BOTTOMLEFT:
                  TextPoint.x = 5;
                  TextPoint.y = rect.height-height;
                  break;
            case DIAL_POSITION_BOTTOMRIGHT:
                  TextPoint.x = rect.width-width-5;
                  TextPoint.y = rect.height-height;
                  break;
      }

      dc->DrawText(text, TextPoint);
}

void DashboardInstrument_Dial::DrawForeground(wxDC* dc)
{
      // The default foreground is the arrow used in most dials
      wxColour cl;
      GetGlobalColor(_T("GREY1"), &cl);
      wxPen pen1;
      pen1.SetStyle(wxSOLID);
      pen1.SetColour(cl);
      pen1.SetWidth(2);
      dc->SetPen(pen1);
      GetGlobalColor(_T("GREY2"), &cl);
      wxBrush brush1;
      brush1.SetStyle(wxSOLID);
      brush1.SetColour(cl);
      dc->SetBrush(brush1);
      dc->DrawCircle(m_cx, m_cy, m_radius / 8);

      dc->SetPen(*wxTRANSPARENT_PEN);

      GetGlobalColor(_T("BLUE1"), &cl);
      wxBrush brush;
      brush.SetStyle(wxSOLID);
      brush.SetColour(cl);
      dc->SetBrush(brush);

      double value = deg2rad((m_MainValue - m_MainValueMin) * m_AngleRange / (m_MainValueMax - m_MainValueMin)) + deg2rad(m_AngleStart - ANGLE_OFFSET);

      wxPoint points[3];
      points[0].x = m_cx + (m_radius * 0.95 * cos(value));
      points[0].y = m_cy + (m_radius * 0.95 * sin(value));
      points[1].x = m_cx + (m_radius * 0.22 * cos(value + 160));
      points[1].y = m_cy + (m_radius * 0.22 * sin(value + 160));
      points[2].x = m_cx + (m_radius * 0.22 * cos(value - 160));
      points[2].y = m_cy + (m_radius * 0.22 * sin(value - 160));
      dc->DrawPolygon(3, points, 0, 0);
}

