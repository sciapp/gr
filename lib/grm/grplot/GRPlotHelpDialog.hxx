#ifndef GRPLOTHELPDIALOG_HXX_INCLUDED
#define GRPLOTHELPDIALOG_HXX_INCLUDED

#include <optional>
#include <string>

#include <QDialog>
#include <QLineEdit>
#include <QTextBrowser>
#include <QWidget>

class GRPlotHelpDialog : public QDialog
{
public:
  explicit GRPlotHelpDialog(QWidget *parent = nullptr, std::optional<std::string> kind = std::nullopt);
  ~GRPlotHelpDialog() override;

protected:
  void keyPressEvent(QKeyEvent *event) override;

private slots:
  void findButtonClickedSlot();

private:
  QLineEdit *find_line_edit;
  QTextBrowser *message;
};

#endif /* ifndef GRPLOTHELPDIALOG_HXX_INCLUDED */
