// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2004 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
/***************************************************************************
    File                 : LayerDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#pragma once

#include "MultiLayer.h"

class QGroupBox;
class QPushButton;
class QSpinBox;
class QCheckBox;
class QComboBox;

//! Arrange layers dialog
class LayerDialog : public QDialog {
  Q_OBJECT

public:
  LayerDialog(QWidget *parent = nullptr, const Qt::WFlags &fl = nullptr);
  void setMultiLayer(MultiLayer *g);

protected slots:
  void accept() override;
  void update();
  void enableLayoutOptions(bool ok);
  void swapLayers();

private:
  MultiLayer *multi_layer;

  QPushButton *buttonOk;
  QPushButton *buttonCancel;
  QPushButton *buttonApply;
  QPushButton *buttonSwapLayers;
  QGroupBox *GroupCanvasSize, *GroupGrid;
  QSpinBox *boxX, *boxY, *boxColsGap, *boxRowsGap;
  QSpinBox *boxRightSpace, *boxLeftSpace, *boxTopSpace, *boxBottomSpace;
  QSpinBox *boxCanvasWidth, *boxCanvasHeight, *layersBox;
  QSpinBox *boxLayerDest, *boxLayerSrc;
  QCheckBox *fitBox;
  QComboBox *alignHorBox, *alignVertBox;
};
