#ifndef ABSTRACTPLUGIN_H
#define ABSTRACTPLUGIN_H

#include <QWidget>
//#include "imageviewer.h"
enum class SliceType;
class SliceItem;
class AbstractSliceDataModel;
class SliceView;
class AbstractPlugin:public QWidget
{
public:
    AbstractPlugin(SliceType type,const QString & name,SliceView * view = nullptr, AbstractSliceDataModel * model = nullptr, QWidget * parent = nullptr);
	virtual  ~AbstractPlugin(){}
protected slots:
	virtual void sliceChanged(int index);
	virtual void sliceSelected(const QPoint & pos);
	virtual void sliceOpened(int index);
	virtual void slicePlayStoped(int index);
	virtual void slicePlaying(int index);
protected:
	SliceItem * sliceItem();
	QImage originalImage(int index);
	SliceView * view();
	QString sliceName()const;
private:
	Q_OBJECT
	AbstractSliceDataModel * m_model;
	SliceView *m_view;
	SliceType m_type;
	QString m_sliceName;

	friend class ImageCanvas;
};

#endif // ABSTRACTPLUGIN_H