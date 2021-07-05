// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2021 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "DllOption.h"
#include "IGLDisplay.h"
#include "IInstrumentDisplay.h"
#include "IQtDisplay.h"

#include <memory>

// Qt forward declarations
class QStackedLayout;
class QWidget;

namespace MantidQt::MantidWidgets {

class EXPORT_OPT_MANTIDQT_INSTRUMENTVIEW InstrumentDisplay : public IInstrumentDisplay {
public:
  InstrumentDisplay(std::unique_ptr<IGLDisplay> glDisplay, std::unique_ptr<IQtDisplay> qtDisplay, QWidget *parent);

  int currentIndex() const override;
  QWidget *currentWidget() const override;
  void setCurrentIndex(int val) const override;

  IGLDisplay *getGLDisplay() const override;
  IQtDisplay *getQtDisplay() const override;

  void installEventFilter(QObject *obj) override;

private:
  QStackedLayout *createLayout(QWidget *parent) const override;

  std::unique_ptr<IGLDisplay> m_glDisplay;
  std::unique_ptr<IQtDisplay> m_qtDisplay;

  /// Stacked layout managing m_glDisplay and m_qtDisplay
  QStackedLayout *m_instrumentDisplayLayout;
};
} // namespace MantidQt::MantidWidgets