#ifndef GIAGUI_DATASETLISTMODEL_HPP
#define GIAGUI_DATASETLISTMODEL_HPP


#include <QAbstractListModel>


struct Dataset;


class DatasetListModel : public QAbstractListModel
{
protected:
	std::vector<Dataset*> items;
	
	
public:
	explicit DatasetListModel(QObject* parent = nullptr);
	~DatasetListModel() override;
	
	int           rowCount() const;
	int           rowCount(const QModelIndex& parent) const override;
	QVariant      data(const QModelIndex& index, int role) const override;
	QVariant      headerData(int section, Qt::Orientation orientation, int role) const override;
	bool          setData(const QModelIndex& index, const QVariant& value, int role) override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	bool          insertRows(int row, int count, const QModelIndex& parent) override;
	bool          removeRows(int row, int count, const QModelIndex& parent) override;
	
	Dataset*    get(const QModelIndex& modelIndex);
	QModelIndex findIndex(Dataset* dataset);
	bool        appendItem(Dataset* dataset);
	bool        removeItem(Dataset* dataset);
	
	using Iterator = decltype(items)::iterator;
	Iterator begin();
	Iterator end();
};

#endif //GIAGUI_DATASETLISTMODEL_HPP