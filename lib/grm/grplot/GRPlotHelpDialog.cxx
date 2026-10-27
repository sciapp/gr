#include <QKeyEvent>
#include <QGridLayout>
#include <QPushButton>

#include "GRPlotHelpDialog.hxx"
#include "Util.hxx"

GRPlotHelpDialog::GRPlotHelpDialog(QWidget *parent, std::optional<std::string> kind)
{

  auto path = util::getEnvVar(W("GRDIR"), W(GRDIR)) + W("/lib");

  find_line_edit = new QLineEdit(this);
  find_line_edit->setPlaceholderText("Search in document");
  auto button = new QPushButton("Find", this);
  QObject::connect(button, &QPushButton::clicked, this, &GRPlotHelpDialog::findButtonClickedSlot);
  auto form = new QGridLayout();
  form->addWidget(find_line_edit, 0, 0);
  form->addWidget(button, 0, 1);

  message = new QTextBrowser(this);
#ifdef _WIN32
  message->setSearchPaths(QStringList(QString::fromStdWString(path)));
#else
  message->setSearchPaths(QStringList(QString::fromStdString(path)));
#endif
  message->setSource(QUrl("../share/doc/grplot/grplot.man.md"));
  message->setReadOnly(true);
  message->setOpenExternalLinks(true);
  message->setAlignment(Qt::AlignTop);
  if (kind)
    {
      std::transform(kind->begin(), kind->end(), kind->begin(), ::toupper);
      if (!message->find(QString::fromStdString(*kind), QTextDocument::FindCaseSensitively))
        fprintf(stderr, "No plot type with the name %s was found.\n", kind->c_str());
    }
  form->addWidget(message, 1, 0, 1, 2);
  this->setLayout(form);
}

GRPlotHelpDialog::~GRPlotHelpDialog() = default;

void GRPlotHelpDialog::findButtonClickedSlot()
{
  auto search_string = find_line_edit->text();
  message->find(search_string, QTextDocument::FindWholeWords);
}

void GRPlotHelpDialog::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) findButtonClickedSlot();
}
