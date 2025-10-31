#pragma once
#include "SettingWidget.h"
#include "OvCore/Global/ServiceLocator.h"
#include "QtWidgets/checkbox.h"
#include <QTreeWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QHBoxLayout>
#include <QHeaderView>

#include <QColorDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QStyleFactory>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <fstream>

namespace MOON {
    std::unordered_map<uintptr_t, std::string> TypeMap = {

    { 671907190918037511, "bool" },

    { 12638226781420530164, "float" },

    { 12638232278978672507, "int" },

    { 8201698316938414196, "float2" },

    { 10283618467285603348, "string" },

    { 12638230079955414429, "double" },

    { 17651624654360340441, "float3" }

    };
	std::vector<std::pair<std::string, uintptr_t>> offsetMap;
	std::vector<uintptr_t>							  offsetType;
	std::unordered_map<std::string, std::vector<int>> group;
	static void ReadOffset() {
		std::ifstream f("C:/Project/zfexternal/Build/bin/Release/zfgridhook.txt");
		std::string	  name;
		uintptr_t	  ptr;
		uintptr_t	  type;
		int			  numnodes;
		int			  numgroups;
		int			  nummembers;
		f >> name >> ptr;
		f >> numnodes;
		offsetMap.resize(numnodes);
		offsetType.resize(numnodes);
		for (int i = 0; i < numnodes; i++)
		{
			f >> offsetMap[i].first >> offsetMap[i].second >> offsetType[i];
		}
		f >> numgroups;
		for (int i = 0; i < numgroups; i++)
		{
			f >> name >> nummembers;
			group[name].resize(nummembers);
			for (int j = 0; j < nummembers; j++)
			{
				f >> group[name][j];
			}
		}
	}
	class SettingWidget::SettingWidgetInternal {
	public:
		SettingWidgetInternal(SettingWidget* tree) :mSelf(tree) {
			ReadOffset();
		}		
		QWidget* createGeneralPage() {
            QScrollArea* scrollArea = new QScrollArea();
            scrollArea->setWidgetResizable(true);
            scrollArea->setStyleSheet("QScrollArea { background-color: #f5f5f5; }");

            QWidget* contentWidget = new QWidget();
            QVBoxLayout* mainLayout = new QVBoxLayout(contentWidget);
            mainLayout->setContentsMargins(30, 30, 30, 30);
            mainLayout->setSpacing(20);

            // 页面标题
            QLabel* titleLabel = new QLabel("基本设置");
            titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #333;");
            mainLayout->addWidget(titleLabel);

            // 基本设置分组
            QGroupBox* generalGroup = new QGroupBox("应用基本配置");
            generalGroup->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: #555; border: 1px solid #ddd; border-radius: 6px; margin-top: 10px; padding: 15px; }"
                "QGroupBox::title { subcontrol-origin: margin; left: 10px; }");

            QFormLayout* generalForm = new QFormLayout(generalGroup);
            generalForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
            generalForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
            generalForm->setSpacing(15);
            generalForm->setContentsMargins(10, 10, 10, 10);

            // 语言选择
            QComboBox* langCombo = new QComboBox();
            langCombo->addItems({ "简体中文", "English", "日本語", "한국어" });
            langCombo->setCurrentIndex(0);
            generalForm->addRow("语言设置:", langCombo);

            // 启动选项
            SlidingCheckBox* startWithSystemCheck = new SlidingCheckBox(mSelf,"test");
            generalForm->addRow("随系统启动:", startWithSystemCheck);

            // 自动更新
            SlidingCheckBox* autoUpdateCheck = new SlidingCheckBox(mSelf, "test");
            autoUpdateCheck->setChecked(true);
            generalForm->addRow("自动更新:", autoUpdateCheck);

            // 下载路径
            QWidget* downloadPathWidget = new QWidget();
            QHBoxLayout* downloadLayout = new QHBoxLayout(downloadPathWidget);
            downloadLayout->setContentsMargins(0, 0, 0, 0);

            QLineEdit* downloadEdit = new QLineEdit("C:/Users/Downloads");
            QPushButton* browseBtn = new QPushButton("浏览...");
            browseBtn->setStyleSheet("padding: 4px 10px;");

            downloadLayout->addWidget(downloadEdit);
            downloadLayout->addWidget(browseBtn);
            generalForm->addRow("下载路径:", downloadPathWidget);

            mainLayout->addWidget(generalGroup);

            // 缓存设置分组
            QGroupBox* cacheGroup = new QGroupBox("缓存管理");
            cacheGroup->setStyleSheet(generalGroup->styleSheet());

            QFormLayout* cacheForm = new QFormLayout(cacheGroup);
            cacheForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
            cacheForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
            cacheForm->setSpacing(15);
            cacheForm->setContentsMargins(10, 10, 10, 10);

            // 缓存大小
            QSlider* cacheSlider = new QSlider(Qt::Horizontal);
            cacheSlider->setRange(50, 1000);
            cacheSlider->setValue(200);
            cacheSlider->setTickInterval(50);
            cacheSlider->setTickPosition(QSlider::TicksBelow);

            QWidget* cacheWidget = new QWidget();
            QHBoxLayout* cacheLayout = new QHBoxLayout(cacheWidget);
            cacheLayout->setContentsMargins(0, 0, 0, 0);
            cacheLayout->addWidget(cacheSlider);

            QLabel* cacheValueLabel = new QLabel("200 MB");
            cacheValueLabel->setMinimumWidth(60);
            cacheLayout->addWidget(cacheValueLabel);

            connect(cacheSlider, &QSlider::valueChanged, [=](int value) {
                cacheValueLabel->setText(QString("%1 MB").arg(value));
                });

            cacheForm->addRow("最大缓存大小:", cacheWidget);

            // 清除缓存按钮
            QPushButton* clearCacheBtn = new QPushButton("清除当前缓存");
            clearCacheBtn->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ddd; padding: 5px 10px; border-radius: 4px;");
            cacheForm->addRow("", clearCacheBtn);

            mainLayout->addWidget(cacheGroup);

            // 底部按钮
            QWidget* btnWidget = new QWidget();
            QHBoxLayout* btnLayout = new QHBoxLayout(btnWidget);
            btnLayout->setSpacing(10);
            btnLayout->setAlignment(Qt::AlignRight);

            QPushButton* resetBtn = new QPushButton("恢复默认");
            QPushButton* applyBtn = new QPushButton("应用");
            QPushButton* okBtn = new QPushButton("确定");

            resetBtn->setStyleSheet("padding: 6px 15px;");
            applyBtn->setStyleSheet("padding: 6px 15px; background-color: #2196F3; color: white; border: none; border-radius: 4px;");
            okBtn->setStyleSheet("padding: 6px 15px; background-color: #4CAF50; color: white; border: none; border-radius: 4px;");

            btnLayout->addWidget(resetBtn);
            btnLayout->addWidget(applyBtn);
            btnLayout->addWidget(okBtn);

            mainLayout->addWidget(btnWidget);
            mainLayout->addStretch();

            scrollArea->setWidget(contentWidget);
            return scrollArea;
		}
        QWidget* createAppearancePage()
        {
            QScrollArea* scrollArea = new QScrollArea();
            scrollArea->setWidgetResizable(true);
            scrollArea->setStyleSheet("QScrollArea { background-color: #f5f5f5; }");

            QWidget* contentWidget = new QWidget();
            QVBoxLayout* mainLayout = new QVBoxLayout(contentWidget);
            mainLayout->setContentsMargins(30, 30, 30, 30);
            mainLayout->setSpacing(20);

            // 页面标题
            QLabel* titleLabel = new QLabel("外观设置");
            titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #333;");
            mainLayout->addWidget(titleLabel);

            // 主题设置分组
            QGroupBox* themeGroup = new QGroupBox("主题与样式");
            themeGroup->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: #555; border: 1px solid #ddd; border-radius: 6px; margin-top: 10px; padding: 15px; }"
                "QGroupBox::title { subcontrol-origin: margin; left: 10px; }");

            QFormLayout* themeForm = new QFormLayout(themeGroup);
            themeForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
            themeForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
            themeForm->setSpacing(15);

            // 主题选择
            QComboBox* themeCombo = new QComboBox();
            themeCombo->addItems({ "浅色主题", "深色主题", "跟随系统" });
            themeForm->addRow("应用主题:", themeCombo);

            // 样式选择
            QComboBox* styleCombo = new QComboBox();
            styleCombo->addItems(QStyleFactory::keys());
            styleCombo->setCurrentText("Fusion");
            themeForm->addRow("界面样式:", styleCombo);

            // 自定义 accent 颜色
            QWidget* colorWidget = new QWidget();
            QHBoxLayout* colorLayout = new QHBoxLayout(colorWidget);
            colorLayout->setContentsMargins(0, 0, 0, 0);

            QLabel* colorLabel = new QLabel();
            colorLabel->setFixedSize(24, 24);
            colorLabel->setStyleSheet("background-color: #2196F3; border-radius: 4px;");

            QPushButton* colorBtn = new QPushButton("选择强调色");
            colorBtn->setStyleSheet("padding: 4px 10px;");

            colorLayout->addWidget(colorLabel);
            colorLayout->addWidget(colorBtn);
            themeForm->addRow("强调色:", colorWidget);

            mainLayout->addWidget(themeGroup);

            // 字体设置分组
            QGroupBox* fontGroup = new QGroupBox("字体设置");
            fontGroup->setStyleSheet(themeGroup->styleSheet());

            QFormLayout* fontForm = new QFormLayout(fontGroup);
            fontForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
            fontForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
            fontForm->setSpacing(15);

            // 字体选择
            QWidget* fontWidget = new QWidget();
            QHBoxLayout* fontLayout = new QHBoxLayout(fontWidget);
            fontLayout->setContentsMargins(0, 0, 0, 0);

            QLabel* fontInfoLabel = new QLabel("Segoe UI, 9pt");
            fontInfoLabel->setStyleSheet("padding: 5px; border: 1px solid #ddd; border-radius: 3px; background-color: white;");
            QPushButton* fontBtn = new QPushButton("选择字体...");
            fontBtn->setStyleSheet("padding: 4px 10px;");

            fontLayout->addWidget(fontInfoLabel, 1);
            fontLayout->addWidget(fontBtn);
            fontForm->addRow("应用字体:", fontWidget);

            // 字体大小
            QSlider* fontSizeSlider = new QSlider(Qt::Horizontal);
            fontSizeSlider->setRange(8, 16);
            fontSizeSlider->setValue(9);
            fontSizeSlider->setTickInterval(1);
            fontSizeSlider->setTickPosition(QSlider::TicksBelow);

            QWidget* fontSizeWidget = new QWidget();
            QHBoxLayout* fontSizeLayout = new QHBoxLayout(fontSizeWidget);
            fontSizeLayout->setContentsMargins(0, 0, 0, 0);
            fontSizeLayout->addWidget(fontSizeSlider);

            QLabel* fontSizeLabel = new QLabel("9 pt");
            fontSizeLabel->setMinimumWidth(40);
            fontSizeLayout->addWidget(fontSizeLabel);

            connect(fontSizeSlider, &QSlider::valueChanged, [=](int value) {
                fontSizeLabel->setText(QString("%1 pt").arg(value));
                });

            fontForm->addRow("字体大小:", fontSizeWidget);

            mainLayout->addWidget(fontGroup);

            // 底部按钮
            QWidget* btnWidget = new QWidget();
            QHBoxLayout* btnLayout = new QHBoxLayout(btnWidget);
            btnLayout->setSpacing(10);
            btnLayout->setAlignment(Qt::AlignRight);

            QPushButton* resetBtn = new QPushButton("恢复默认");
            QPushButton* applyBtn = new QPushButton("应用");
            QPushButton* okBtn = new QPushButton("确定");

            resetBtn->setStyleSheet("padding: 6px 15px;");
            applyBtn->setStyleSheet("padding: 6px 15px; background-color: #2196F3; color: white; border: none; border-radius: 4px;");
            okBtn->setStyleSheet("padding: 6px 15px; background-color: #4CAF50; color: white; border: none; border-radius: 4px;");

            btnLayout->addWidget(resetBtn);
            btnLayout->addWidget(applyBtn);
            btnLayout->addWidget(okBtn);

            mainLayout->addWidget(btnWidget);
            mainLayout->addStretch();

            scrollArea->setWidget(contentWidget);
            return scrollArea;
        }

        QWidget* createNotificationsPage()
        {
            QScrollArea* scrollArea = new QScrollArea();
            scrollArea->setWidgetResizable(true);
            scrollArea->setStyleSheet("QScrollArea { background-color: #f5f5f5; }");

            QWidget* contentWidget = new QWidget();
            QVBoxLayout* mainLayout = new QVBoxLayout(contentWidget);
            mainLayout->setContentsMargins(30, 30, 30, 30);
            mainLayout->setSpacing(20);

            // 页面标题
            QLabel* titleLabel = new QLabel("通知设置");
            titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #333;");
            mainLayout->addWidget(titleLabel);

            // 通知开关总控
            SlidingCheckBox* enableAllCheck = new SlidingCheckBox();
            enableAllCheck->setChecked(true);

            QWidget* globalNotifyWidget = new QWidget();
            QHBoxLayout* globalNotifyLayout = new QHBoxLayout(globalNotifyWidget);
            globalNotifyLayout->setContentsMargins(0, 0, 0, 0);

            QLabel* globalNotifyLabel = new QLabel("启用所有通知");
            globalNotifyLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
            globalNotifyLayout->addWidget(globalNotifyLabel);
            globalNotifyLayout->addStretch();
            globalNotifyLayout->addWidget(enableAllCheck);

            mainLayout->addWidget(globalNotifyWidget);

            // 通知类型分组
            QGroupBox* notifyGroup = new QGroupBox("通知类型");
            notifyGroup->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: #555; border: 1px solid #ddd; border-radius: 6px; margin-top: 10px; padding: 15px; }"
                "QGroupBox::title { subcontrol-origin: margin; left: 10px; }");

            QVBoxLayout* notifyLayout = new QVBoxLayout(notifyGroup);
            notifyLayout->setSpacing(15);

            // 各种通知选项
            for (const QString& notifyType : { "新消息通知", "系统更新提醒", "任务完成提示",
                                              "好友请求通知", "日历事件提醒", "文件传输通知" }) {
                QWidget* itemWidget = new QWidget();
                QHBoxLayout* itemLayout = new QHBoxLayout(itemWidget);
                itemLayout->setContentsMargins(0, 0, 0, 0);

                QLabel* label = new QLabel(notifyType);
                SlidingCheckBox* checkBox = new SlidingCheckBox();
                checkBox->setChecked(true);

                // 绑定总开关状态
                connect(enableAllCheck, &SlidingCheckBox::toggled, checkBox, &SlidingCheckBox::setEnabled);

                itemLayout->addWidget(label);
                itemLayout->addStretch();
                itemLayout->addWidget(checkBox);

                notifyLayout->addWidget(itemWidget);
            }

            mainLayout->addWidget(notifyGroup);

            // 通知样式分组
            QGroupBox* styleGroup = new QGroupBox("通知样式");
            styleGroup->setStyleSheet(notifyGroup->styleSheet());

            QFormLayout* styleForm = new QFormLayout(styleGroup);
            styleForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
            styleForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
            styleForm->setSpacing(15);

            // 通知位置
            QComboBox* positionCombo = new QComboBox();
            positionCombo->addItems({ "右上角", "右下角", "左上角", "左下角" });
            positionCombo->setCurrentIndex(0);
            styleForm->addRow("通知位置:", positionCombo);

            // 通知持续时间
            QComboBox* durationCombo = new QComboBox();
            durationCombo->addItems({ "3秒", "5秒", "10秒", "30秒", "不自动关闭" });
            durationCombo->setCurrentIndex(1);
            styleForm->addRow("显示时长:", durationCombo);

            // 通知音效
            SlidingCheckBox* soundCheck = new SlidingCheckBox();
            soundCheck->setChecked(true);
            styleForm->addRow("通知音效:", soundCheck);

            // 震动提醒（针对移动设备）
            SlidingCheckBox* vibrateCheck = new SlidingCheckBox();
            styleForm->addRow("震动提醒:", vibrateCheck);

            mainLayout->addWidget(styleGroup);

            // 底部按钮
            QWidget* btnWidget = new QWidget();
            QHBoxLayout* btnLayout = new QHBoxLayout(btnWidget);
            btnLayout->setSpacing(10);
            btnLayout->setAlignment(Qt::AlignRight);

            QPushButton* resetBtn = new QPushButton("恢复默认");
            QPushButton* applyBtn = new QPushButton("应用");
            QPushButton* okBtn = new QPushButton("确定");

            resetBtn->setStyleSheet("padding: 6px 15px;");
            applyBtn->setStyleSheet("padding: 6px 15px; background-color: #2196F3; color: white; border: none; border-radius: 4px;");
            okBtn->setStyleSheet("padding: 6px 15px; background-color: #4CAF50; color: white; border: none; border-radius: 4px;");

            btnLayout->addWidget(resetBtn);
            btnLayout->addWidget(applyBtn);
            btnLayout->addWidget(okBtn);

            mainLayout->addWidget(btnWidget);
            mainLayout->addStretch();

            scrollArea->setWidget(contentWidget);
            return scrollArea;
        }

        QWidget* createAccountPage()
        {
            // 实现账户设置页面（类似其他页面结构）
            QScrollArea* scrollArea = new QScrollArea();
            scrollArea->setWidgetResizable(true);
            scrollArea->setStyleSheet("QScrollArea { background-color: #f5f5f5; }");

            QWidget* contentWidget = new QWidget();
            QVBoxLayout* mainLayout = new QVBoxLayout(contentWidget);
            mainLayout->setContentsMargins(30, 30, 30, 30);
            mainLayout->setSpacing(20);

            // 页面标题
            QLabel* titleLabel = new QLabel("账户设置");
            titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #333;");
            mainLayout->addWidget(titleLabel);

            // 账户信息分组
            QGroupBox* accountGroup = new QGroupBox("账户信息");
            accountGroup->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: #555; border: 1px solid #ddd; border-radius: 6px; margin-top: 10px; padding: 15px; }"
                "QGroupBox::title { subcontrol-origin: margin; left: 10px; }");

            QFormLayout* accountForm = new QFormLayout(accountGroup);
            accountForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
            accountForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
            accountForm->setSpacing(15);

            // 用户名
            QLineEdit* usernameEdit = new QLineEdit("user123");
            usernameEdit->setReadOnly(true);
            accountForm->addRow("用户名:", usernameEdit);

            // 邮箱
            QLineEdit* emailEdit = new QLineEdit("user@example.com");
            accountForm->addRow("电子邮箱:", emailEdit);

            // 姓名
            QLineEdit* nameEdit = new QLineEdit("张三");
            accountForm->addRow("姓名:", nameEdit);

            mainLayout->addWidget(accountGroup);

            // 安全设置分组
            QGroupBox* securityGroup = new QGroupBox("安全设置");
            securityGroup->setStyleSheet(accountGroup->styleSheet());

            QFormLayout* securityForm = new QFormLayout(securityGroup);
            securityForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
            securityForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
            securityForm->setSpacing(15);

            // 更改密码
            QPushButton* pwdBtn = new QPushButton("更改密码");
            pwdBtn->setStyleSheet("padding: 4px 10px;");
            securityForm->addRow("密码:", pwdBtn);

            // 两步验证
            SlidingCheckBox* twoFactorCheck = new SlidingCheckBox();
            securityForm->addRow("两步验证:", twoFactorCheck);

            // 登录历史
            QPushButton* historyBtn = new QPushButton("查看登录历史");
            historyBtn->setStyleSheet("padding: 4px 10px;");
            securityForm->addRow("登录记录:", historyBtn);

            mainLayout->addWidget(securityGroup);

            // 底部按钮
            QWidget* btnWidget = new QWidget();
            QHBoxLayout* btnLayout = new QHBoxLayout(btnWidget);
            btnLayout->setSpacing(10);
            btnLayout->setAlignment(Qt::AlignRight);

            QPushButton* cancelBtn = new QPushButton("取消");
            QPushButton* saveBtn = new QPushButton("保存更改");
            saveBtn->setStyleSheet("padding: 6px 15px; background-color: #4CAF50; color: white; border: none; border-radius: 4px;");

            btnLayout->addWidget(cancelBtn);
            btnLayout->addWidget(saveBtn);

            mainLayout->addWidget(btnWidget);
            mainLayout->addStretch();

            scrollArea->setWidget(contentWidget);
            return scrollArea;
        }

        QWidget* createAdvancedPage()
        {
            // 实现高级设置页面
            QScrollArea* scrollArea = new QScrollArea();
            scrollArea->setWidgetResizable(true);
            scrollArea->setStyleSheet("QScrollArea { background-color: #f5f5f5; }");

            QWidget* contentWidget = new QWidget();
            QVBoxLayout* mainLayout = new QVBoxLayout(contentWidget);
            mainLayout->setContentsMargins(30, 30, 30, 30);
            mainLayout->setSpacing(20);

            // 页面标题
            QLabel* titleLabel = new QLabel("高级设置");
            titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #333;");
            mainLayout->addWidget(titleLabel);

            // 警告提示
            QLabel* warningLabel = new QLabel("警告：修改以下设置可能会影响应用正常运行，仅建议高级用户调整。");
            warningLabel->setStyleSheet("color: #e53935; font-size: 13px; background-color: #ffebee; padding: 8px; border-radius: 4px;");
            warningLabel->setWordWrap(true);
            mainLayout->addWidget(warningLabel);

            // 高级选项分组
            QGroupBox* advancedGroup = new QGroupBox("系统选项");
            advancedGroup->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: #555; border: 1px solid #ddd; border-radius: 6px; margin-top: 10px; padding: 15px; }"
                "QGroupBox::title { subcontrol-origin: margin; left: 10px; }");

            QFormLayout* advancedForm = new QFormLayout(advancedGroup);
            advancedForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
            advancedForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
            advancedForm->setSpacing(15);

            // 硬件加速
            SlidingCheckBox* hardwareAccelCheck = new SlidingCheckBox();
            hardwareAccelCheck->setChecked(true);
            advancedForm->addRow("启用硬件加速:", hardwareAccelCheck);

            // 日志级别
            QComboBox* logLevelCombo = new QComboBox();
            logLevelCombo->addItems({ "错误", "警告", "信息", "调试", "详细" });
            logLevelCombo->setCurrentIndex(2);
            advancedForm->addRow("日志级别:", logLevelCombo);

            // 代理设置
            QComboBox* proxyCombo = new QComboBox();
            proxyCombo->addItems({ "无代理", "系统代理", "手动设置" });
            advancedForm->addRow("代理设置:", proxyCombo);

            mainLayout->addWidget(advancedGroup);

            // 数据管理分组
            QGroupBox* dataGroup = new QGroupBox("数据管理");
            dataGroup->setStyleSheet(advancedGroup->styleSheet());

            QVBoxLayout* dataLayout = new QVBoxLayout(dataGroup);
            dataLayout->setSpacing(15);

            QPushButton* exportBtn = new QPushButton("导出用户数据");
            QPushButton* importBtn = new QPushButton("导入用户数据");
            QPushButton* resetBtn = new QPushButton("重置应用数据");
            resetBtn->setStyleSheet("color: #e53935;");

            dataLayout->addWidget(exportBtn);
            dataLayout->addWidget(importBtn);
            dataLayout->addWidget(resetBtn);

            mainLayout->addWidget(dataGroup);

            // 底部按钮
            QWidget* btnWidget = new QWidget();
            QHBoxLayout* btnLayout = new QHBoxLayout(btnWidget);
            btnLayout->setSpacing(10);
            btnLayout->setAlignment(Qt::AlignRight);

            QPushButton* cancelBtn = new QPushButton("取消");
            QPushButton* applyBtn = new QPushButton("应用");
            applyBtn->setStyleSheet("padding: 6px 15px; background-color: #2196F3; color: white; border: none; border-radius: 4px;");

            btnLayout->addWidget(cancelBtn);
            btnLayout->addWidget(applyBtn);

            mainLayout->addWidget(btnWidget);
            mainLayout->addStretch();

            scrollArea->setWidget(contentWidget);
            return scrollArea;
        }

		void setUp() {
			// 主布局
			QHBoxLayout* mainLayout = new QHBoxLayout(mSelf);
			mainLayout->setContentsMargins(0, 0, 0, 0);
			mainLayout->setSpacing(0);
			// 左侧导航树
			m_navTree = new QTreeWidget(mSelf);
			m_navTree->setHeaderHidden(true);
			m_navTree->setMaximumWidth(200);
			m_navTree->setMinimumWidth(180);
			m_navTree->setStyleSheet("QTreeWidget { border-right: 1px solid #ddd; }"
				"QTreeWidget::item { height: 30px; padding-left: 10px; }"
				"QTreeWidget::item:selected { background-color: #e6f7ff; color: #1890ff; }");

			// 添加导航项
            for (auto& g : group) {
				QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << QString::fromStdString(g.first));
				m_navTree->addTopLevelItem(item);
            }


			// 右侧内容区域
			m_contentStack = new QStackedWidget(mSelf);

			// 添加各个设置页面
            for (auto& g : group) {
                // 实现高级设置页面
                QScrollArea* scrollArea = new QScrollArea();
                scrollArea->setWidgetResizable(true);
                scrollArea->setStyleSheet("QScrollArea { background-color: #f5f5f5; }");

                QWidget* contentWidget = new QWidget();
                QVBoxLayout* vLayout = new QVBoxLayout(contentWidget);
                vLayout->setContentsMargins(0, 0, 0, 0);
                vLayout->setSpacing(0);
                vLayout->setAlignment(Qt::AlignTop);

                // 高级选项分组
                QWidget* advancedGroup = new QWidget();
                //advancedGroup->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: #555; border: 1px solid #ddd; border-radius: 6px; margin-top: 10px; padding: 15px; }"
                    ///"QGroupBox::title { subcontrol-origin: margin; left: 10px; }");

                QFormLayout* form = new QFormLayout(advancedGroup);
                form->setRowWrapPolicy(QFormLayout::DontWrapRows);
                form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
                form->setSpacing(15);

                form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
                // 2. 设置字段的对齐方式（顶部对齐）
                form->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
                // 3. 可选：设置行内间距（避免控件太紧凑）
                form->setVerticalSpacing(10); // 行与行之间的垂直间距
                form->setContentsMargins(20, 20, 20, 20); // 布局边缘间距
                for (int idx : g.second) {
                    if (idx < 0 || idx >= offsetMap.size()) continue;
                    const auto& [name, offset] = offsetMap[idx];
                    QLabel* label = new QLabel(QString::fromStdString(name));
                    if (TypeMap[offsetType[idx]] == "bool") {
						SlidingCheckBox* checkBox = new SlidingCheckBox();
						form->addRow(label, checkBox);
                    }
                    else if (TypeMap[offsetType[idx]] == "float") {
                        QSlider* cacheSlider = new QSlider(Qt::Horizontal);
                        cacheSlider->setRange(50, 1000);
                        cacheSlider->setValue(200);
                        cacheSlider->setTickInterval(50);
                        cacheSlider->setTickPosition(QSlider::TicksBelow);
						
						form->addRow(label, cacheSlider);
                    }
                    else if (TypeMap[offsetType[idx]] == "int") {
                        QSlider* cacheSlider = new QSlider(Qt::Horizontal);
                        cacheSlider->setRange(50, 1000);
                        cacheSlider->setValue(200);
                        cacheSlider->setTickInterval(50);
                        cacheSlider->setTickPosition(QSlider::TicksBelow);
                        form->addRow(label, cacheSlider);
                    }
                    else {
                       form->addRow(label);
                    }
                   
					
                }
                vLayout->addWidget(advancedGroup);
                scrollArea->setWidget(contentWidget);
				m_contentStack->addWidget(scrollArea);
            }

			// 添加到主布局
			mainLayout->addWidget(m_navTree);
			mainLayout->addWidget(m_contentStack, 1);
			// 连接导航切换信号
			connect(m_navTree, &QTreeWidget::currentItemChanged,
				mSelf, &SettingWidget::onItemChanged);
		}
		~SettingWidgetInternal() {
		}
	private:
		friend class SettingWidget;
		SettingWidget* mSelf = nullptr;
		// 创建各个设置页面

		// 左侧导航树
		QTreeWidget* m_navTree;

		// 右侧内容区域
		QStackedWidget* m_contentStack;

	};
	SettingWidget::SettingWidget(QWidget* parent):QWidget(parent),mInternal(new SettingWidgetInternal(this))
	{
		mInternal->setUp();

	}
	SettingWidget::~SettingWidget()
	{
		delete mInternal;
	}
	void SettingWidget::onItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous) {
		Q_UNUSED(previous);
		int index = mInternal->m_navTree->indexOfTopLevelItem(current);
		if (index >= 0) {
			mInternal->m_contentStack->setCurrentIndex(index);
		}
	}

}