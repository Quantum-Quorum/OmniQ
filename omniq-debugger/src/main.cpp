//
// Created by Goutham Arcot on 17/07/25.
//

#include "MainWindow.h"
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QStyleFactory>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // Set application properties
  app.setApplicationName("OmniQ Debugger");
  app.setApplicationVersion("0.1.0");
  app.setOrganizationName("Quantum-Quorum");
  app.setOrganizationDomain("quantum-quorum.com");

  // Set modern style
  app.setStyle(QStyleFactory::create("Fusion"));

  // Create and show main window
  MainWindow window;

  // Load circuit if passed as argument
  if (argc > 1) {
    window.loadCircuit(argv[1]);
  }

  window.show();

  return app.exec();
}
