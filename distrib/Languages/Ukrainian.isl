; *** Inno Setup version 5.1.11+ Ukrainian messages ***
;
; Translation was made by Oleg Romanyk, olegu4ok@ya.ru
; The highest accuracy was the first priority.
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).
;
; $jrsoftware: issrc/Files/Languages/Ukrainian.isl, ver. 2007.12.29 16:34:40 jr Exp $

[LangOptions]
; The following three entries are very important. Be sure to read and 
; understand the '[LangOptions] section' topic in the help file.
LanguageName=<0423><043A><0440><0430><0457><043D><0441><044C><043A><0430>
LanguageID=$0422
LanguageCodePage=1251
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
;DialogFontName=
;DialogFontSize=8
;WelcomeFontName=Verdana
;WelcomeFontSize=12
;TitleFontName=Arial
;TitleFontSize=29
;CopyrightFontName=Arial
;CopyrightFontSize=8

[Messages]

; *** Application titles
SetupAppTitle=Інсталяція
SetupWindowTitle=Інсталяція — %1
UninstallAppTitle=Деінсталяція
UninstallAppFullTitle=Деінсталяція — %1

; *** Misc. common
InformationTitle=Інформація
ConfirmTitle=Підтвердження
ErrorTitle=Помилка

; *** SetupLdr messages
SetupLdrStartupMessage=Ця програма встановить %1 на Ваш комп'ютер, продовжити?
LdrCannotCreateTemp=Неможливо створити тимчасовий файл. Інсталяція перервана
LdrCannotExecTemp=Неможливо виконати файл у тимчасовому каталозі. Інсталяція перервана

; *** Startup error messages
LastErrorMessage=%1.%n%nПомилка %2: %3
SetupFileMissing=Файл %1 відсутній в папці інсталяції. Будь ласка, усуньте проблему або отримайте нову версію програми.
SetupFileCorrupt=Інсталяційні файли пошкодженні. Будь ласка, отримайте нову копію програми.
SetupFileCorruptOrWrongVer=Ці інсталяційні файли пошкодженні або несумісні з даною версією програми інсталяцій. Будь ласка, усуньте проблему або отримайте нову копію програми.
NotOnThisPlatform=Ця програма не буде працювати у %1.
OnlyOnThisPlatform=Цю програму можна запускати тільки у %1.
OnlyOnTheseArchitectures=Інсталяція цієї програми можлива тільки у версіях Windows для наступних архітектур процесорів:%n%n%1
MissingWOW64APIs=У версії Windows, в якій Ви працюєте, відсутні функції, необхідні для виконання 64-бітної інсталяції. Щоб усунути цю проблему, Вам необхідно встановити пакет оновлень (Service Pack) %1.
WinVersionTooLowError=Ця програма потребує %1 версії %2 або вище.
WinVersionTooHighError=Програма не може бути встановлена у %1 версії %2 або вище.
AdminPrivilegesRequired=Щоб встановити цю програму, Вам потрібно виконати вхід у систему як Адміністратор.
PowerUserPrivilegesRequired=Щоб встановити цю програму, Ви повинні виконати вхід у систему як Адміністратор або член групи "Досвідчені користувачі" (Power Users).
SetupAppRunningError=Виявлено запущений екземпляр %1.%n%nБудь ласка, закрийте усі екземпляри додатку, потім натисніть «OK», щоб продовжити, або «Скасувати», щоб вийти.
UninstallAppRunningError=Деінсталятор виявив запущений екземпляр %1.%n%nБудь ласка, закрийте усі екземпляри додатку, потім натисніть «OK», щоб продовжити, або «Скасувати», щоб вийти.

; *** Misc. errors
ErrorCreatingDir=Неможливо створити папку "%1"
ErrorTooManyFilesInDir=Неможливо створити файл в папці "%1", так як в ній занадто багато файлів

; *** Setup common messages
ExitSetupTitle=Вихід із програми інсталяції
ExitSetupMessage=Встановлення не завершено. Якщо Ви вийдете, програма не буде встановлена.%n%nВи зможете завершити встановлення, запустивши програму інсталяції пізніше.%n%nВийти із програми інсталяції?
AboutSetupMenuItem=&Про програму...
AboutSetupTitle=Про програму
AboutSetupMessage=%1, версія %2%n%3%n%nСайт %1:%n%4
AboutSetupNote=
TranslatorNote=Ukrainian translation by Oleg Romanyk, olegu4ok@ya.ru

; *** Buttons
ButtonBack=< &Назад
ButtonNext=&Далі >
ButtonInstall=&Встановити
ButtonOK=OK
ButtonCancel=Скасувати
ButtonYes=&Так
ButtonYesToAll=Так для &Всіх
ButtonNo=&Ні
ButtonNoToAll=Н&і для Всіх
ButtonFinish=&Завершити
ButtonBrowse=&Огляд...
ButtonWizardBrowse=&Огляд...
ButtonNewFolder=&Створити папку

; *** "Select Language" dialog messages
SelectLanguageTitle=Виберіть мову інсталяції
SelectLanguageLabel=Виберіть мову, яка буде використовуватися в процесі інсталяції:

; *** Common wizard text
ClickNext=Натисніть «Далі», щоб продовжити, або «Скасувати», щоб вийти із програми інсталяції.
BeveledLabel=
BrowseDialogTitle=Огляд папок
BrowseDialogLabel=Виберіть папку із списку і натисніть «ОК».
NewFolderName=Нова папка

; *** "Welcome" wizard page
WelcomeLabel1=Ласкаво просимо до Майстра інсталяції [name]
WelcomeLabel2=Програма встановить [name/ver] на Ваш комп'ютер.%n%nРекомендуємо закрити всі інші програми перед тим, як продовжити.

; *** "Password" wizard page
WizardPassword=Пароль
PasswordLabel1=Ця програма захищена паролем.
PasswordLabel3=Будь ласка, введіть пароль, потім натисніть «Далі». Паролі необхідно вводити з врахуванням реєстру.
PasswordEditLabel=&Пароль:
IncorrectPassword=Введений вами пароль невірний. Будь ласка, спробуйте ще раз.

; *** "License Agreement" wizard page
WizardLicense=Ліцензійна Угода
LicenseLabel=Будь ласка, прочитайте наступну важливу інформацію перед тим, як продовжити.
LicenseLabel3=Будь ласка, прочитайте наступну Ліцензійну Угоду. Ви повинні прийняти умови угоди перед тим, як продовжити.
LicenseAccepted=Я &приймаю умови угоди
LicenseNotAccepted=Я &не приймаю умови угоди

; *** "Information" wizard pages
WizardInfoBefore=Інформація
InfoBeforeLabel=Будь ласка, прочитайте наступну важливу інформацію перед тим, як продовжити.
InfoBeforeClickLabel=Коли Ви будете готові продовжити інсталяцію, натисніть «Далі».
WizardInfoAfter=Інформація
InfoAfterLabel=Будь ласка, прочитайте наступну важливу інформацію перед тим, як продовжити.
InfoAfterClickLabel=Коли Ви будете готові продовжити інсталяцію, натисніть «Далі».

; *** "User Information" wizard page
WizardUserInfo=Інформація про користувача
UserInfoDesc=Будь ласка, введіть дані про себе.
UserInfoName=&Ім'я і прізвище користувача:
UserInfoOrg=&Організація:
UserInfoSerial=&Серійний номер:
UserInfoNameRequired=Ви повинні ввести ім'я.

; *** "Select Destination Location" wizard page
WizardSelectDir=Вибір папки встановлення
SelectDirDesc=У яку папку Ви хочете встановити [name]?
SelectDirLabel3=Програма встановить [name] у наступну папку.
SelectDirBrowseLabel=Натисніть «Далі», щоб продовжити. Якщо Ви хочете вибрати іншу папку, натисніть «Огляд...».
DiskSpaceMBLabel=Потрібно як мінімум [mb] Мб вільного дискового простору.
ToUNCPathname=Інсталяція не може виконуватися в папку по її мережевому імені. Якщо Ви інсталюєте в мережеву папку, Ви повинні під'єднати її у вигляді мережевого диску.
InvalidPath=Ви повинні вказати повний шлях з буквою диску; %nнаприклад:%n%nC:\APP%n%nабо у форматі UNC:%n%n\\ім'я_сервера\ім'я_ресурсу
InvalidDrive=Вибраний Вами диск чи мережевий шлях не існує або недоступний. Будь ласка, виберіть інший.
DiskSpaceWarningTitle=Недостатньо простору на диску
DiskSpaceWarning=Інсталяція потребує не менше %1 КБ вільного простору, а на вибраному Вами диску тільки %2 КБ.%n%nВи бажаєте тим паче продовжити інсталяцію?
DirNameTooLong=Ім'я папки чи шлях до неї перевищують припустиму довжину.
InvalidDirName=Вказане ім'я папки неприпустиме.
BadDirName32=Ім'я папки не може містити таких символів: %n%n%1
DirExistsTitle=Папка існує
DirExists=Папка%n%n%1%n%nвже існує. Все рівно встановити в цю папку?
DirDoesntExistTitle=Папка не існує
DirDoesntExist=Папка%n%n%1%n%nне існує. Ви хочете створити її?

; *** "Select Components" wizard page
WizardSelectComponents=Вибір компонентів
SelectComponentsDesc=Які компоненти повинні бути встановлені?
SelectComponentsLabel2=Виберіть компоненти, які потрібно встановити; зніміть прапорці з компонентів, встановлювати котрі не потрібно. Натисніть «Далі», коли Ви будете готові продовжити.
FullInstallation=Повна інсталяція
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=Компактна інсталяція
CustomInstallation=Вибіркова інсталяція
NoUninstallWarningTitle=Встановлені компоненти
NoUninstallWarning=Програма інсталяції виявила, що наступні компоненти вже встановлені на Вашому комп'ютері:%n%n%1%n%nСкасування вибору цих компонентів не видалить їх.%n%nПродовжити?
ComponentSize1=%1 КБ
ComponentSize2=%1 МБ
ComponentsDiskSpaceMBLabel=Поточний вибір потребує не менше [mb] Мб на диску.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=Виберіть додаткові завдання
SelectTasksDesc=Які додаткові завдання необхідно виконати?
SelectTasksLabel2=Виберіть додаткові завдання, котрі повинні виконати про інсталяції [name], після цього натисніть «Далі».

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=Виберіть папку в меню «Старт»
SelectStartMenuFolderDesc=Де програма інсталяції повинна створити ярлики?
SelectStartMenuFolderLabel3=Програма інсталяції створить ярлики в наступній папці меню «Старт».
SelectStartMenuFolderBrowseLabel=Натисніть «Далі», щоб продовжити. Якщо Ви хочете вибрати іншу папку, натисніть «Огляд...».
MustEnterGroupName=Ви повинні ввести ім'я папки.
GroupNameTooLong=Ім'я папки чи шлях до неї перевищують припустиму довжину.
InvalidGroupName=Вказане ім'я папки неприпустиме.
BadGroupName=Ім'я папки не може містити символів:%n%n%1
NoProgramGroupCheck2=&Не створювати папку в меню «Старт»

; *** "Ready to Install" wizard page
WizardReady=Все готово для інсталяції
ReadyLabel1=Програма інсталяції готова почати встановлення [name] на Ваш комп'ютер.
ReadyLabel2a=Натисніть «Встановити», щоб продовжити, або «Назад», якщо ви хочете продивитися чи змінити опції інсталяції.
ReadyLabel2b=Натисніть «Встановити», щоб продовжити.
ReadyMemoUserInfo=Інформація про користувача:
ReadyMemoDir=Папка інсталяції:
ReadyMemoType=Тип інсталяції:
ReadyMemoComponents=Вибрані компоненти:
ReadyMemoGroup=Папка в меню «Старт»:
ReadyMemoTasks=Додаткові завдання:

; *** "Preparing to Install" wizard page
WizardPreparing=Підготовка до інсталяції
PreparingDesc=Програма інсталяції готується до встановлення [name] на Ваш комп'ютер.
PreviousInstallNotCompleted=Інсталяція або деінсталяція попередньої програми не були завершені. Вам знадобиться перезавантажити Ваш комп'ютер, щоби завершити цю інсталяцію.%n%nПісля перезавантаження запустіть знову Програму інсталяції, щоби завершити встановлення [name].
CannotContinue=Неможливо продовжити інсталяцію. Натисніть «Скасувати» для виходу із програми.

; *** "Installing" wizard page
WizardInstalling=Інсталяція...
InstallingLabel=Будь ласка, почекайте, поки [name] встановиться на Ваш комп'ютер.

; *** "Setup Completed" wizard page
FinishedHeadingLabel=Завершення Майстра інсталяції [name]
FinishedLabelNoIcons=Програма [name] встановлена на Ваш комп'ютер.
FinishedLabel=Програма [name] встановлена на Ваш комп'ютер. Додаток можна запустити з допомогою відповідного значка.
ClickFinish=Натисніть «Завершити», щоби вийти із програми інсталяції.
FinishedRestartLabel=Для завершення інсталяції [name] потребується перезавантажити комп'ютер. Перезавантажити зараз?
FinishedRestartMessage=Для завершення інсталяції [name] потребується перезавантажити комп'ютер.%n%nПерезавантажити зараз?
ShowReadmeCheck=Я хочу переглянути файл README
YesRadio=&Так, перезавантажити комп'ютер зараз
NoRadio=&Ні, я перезавантажу комп'ютер пізніше
; used for example as 'Run MyProg.exe'
RunEntryExec=Запустити %1
; used for example as 'View Readme.txt'
RunEntryShellExec=Переглянути %1

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=Необхідно вставити наступний диск
SelectDiskLabel2=Будь ласка, вставте диск %1 і натисніть «OK».%n%nЯкщо файли цього диска можуть бути знайдені в папці, яка відрізняється від показаної нижче, введіть правильний шлях або натисніть «Огляд...».
PathLabel=&Шлях:
FileNotInDir2=Файл "%1" не знайдений у "%2". Будь ласка, вставте правильний диск або виберіть іншу папку.
SelectDirectoryLabel=Будь ласка, вкажіть шлях до наступного диску.

; *** Installation phase messages
SetupAborted=Інсталяція була не завершена.%n%nБудь ласка, усуньте проблему і запустіть інсталяцію знову.
EntryAbortRetryIgnore=Натисніть «Повтор», щоби повторити спробу, «Пропустити», щоби пропустити файл, або «Скасувати» для скасування інсталяції.

; *** Installation status messages
StatusCreateDirs=Створення папок...
StatusExtractFiles=Розпакування файлів...
StatusCreateIcons=Створення ярликів...
StatusCreateIniEntries=Створення INI-файлів...
StatusCreateRegistryEntries=Створення записів реєстру...
StatusRegisterFiles=Реєстрація файлів...
StatusSavingUninstall=Збереження інформації для деінсталяції...
StatusRunProgram=Завершення інсталяції...
StatusRollback=Скасування змін...

; *** Misc. errors
ErrorInternal2=Внутрішня помилка: %1
ErrorFunctionFailedNoCode=%1: збій
ErrorFunctionFailed=%1: збій; код %2
ErrorFunctionFailedWithMessage=%1: збій; код %2.%n%3
ErrorExecutingProgram=Неможливо виконати файл:%n%1

; *** Registry errors
ErrorRegOpenKey=Помилка відкриття ключа реєстру:%n%1\%2
ErrorRegCreateKey=Помилка створення ключа реєстру:%n%1\%2
ErrorRegWriteKey=Помилка запису в ключ реєстру:%n%1\%2

; *** INI errors
ErrorIniEntry=Помилка створення запису в INI-файлі "%1".

; *** File copying errors
FileAbortRetryIgnore=Натисніть «Повтор», щоби повторити, «Пропустити», щоби пропустити файл (не рекомендується) або «Скасувати» для виходу.
FileAbortRetryIgnore2=Натисніть «Повтор», щоби повторити, «Пропустити», щоби ігнорувати помилку (не рекомендується) або «Скасувати» для виходу.
SourceIsCorrupted=Вихідний файл пошкоджений
SourceDoesntExist=Вихідний файл "%1" не існує
ExistingFileReadOnly=Існуючий файл помічений як «файл тільки для читання».%n%nНатисніть «Повтор», щоби видалити атрибут «тільки для читання», «Пропустити», щоби пропустити файл або «Скасувати» для виходу.
ErrorReadingExistingDest=Відбулася помилка при спробі читання існуючого файлу:
FileExists=Файл вже існує.%n%nПерезаписати його?
ExistingFileNewer=Існуючий файл більш новий, чим встановлюваний. Рекомендується зберегти існуючий файл.%n%nВи хочете зберегти існуючий файл?
ErrorChangingAttr=Відбулася помилка при спробі зміни атрибутів існуючого файлу:
ErrorCreatingTemp=Відбулася помилка при спробі створення файлу в папці призначення:
ErrorReadingSource=Відбулася помилка при спробі читання вихідного файлу:
ErrorCopying=Відбулася помилка при спробі копіювання файлу:
ErrorReplacingExistingFile=Відбулася помилка при спробі заміни існуючого файлу:
ErrorRestartReplace=Помилка RestartReplace:
ErrorRenamingTemp=Відбулася помилка при спробі перейменовування файлу в папці призначення:
ErrorRegisterServer=Неможливо зареєструвати DLL/OCX: %1
ErrorRegSvr32Failed=Помилка при виконанні RegSvr32, код повернення %1
ErrorRegisterTypeLib=Неможливо зареєструвати бібліотеку типів (Type Library): %1

; *** Post-installation errors
ErrorOpeningReadme=Відбулася помилка при спробі відкриття файлу README.
ErrorRestartingComputer=Програмі інсталяцій не вдалося перезавантажити комп'ютер. Будь ласка, виконайте це самостійно.

; *** Uninstaller messages
UninstallNotFound=Файл "%1" не існує, деінсталяція неможлива.
UninstallOpenError=Неможливо відкрити файл "%1". Деінсталяція неможлива
UninstallUnsupportedVer=Файл протоколу для деінсталяції "%1" не розпізнаний даною версією програми-деінсталяції. Деінсталяція неможлива
UninstallUnknownEntry=Зустрівся невідомий пункт (%1) в файлі протоколу для деінсталяції
ConfirmUninstall=Ви впевнені, що хочете видалити %1 і всі компоненти програми?
UninstallOnlyOnWin64=Цю програму можливо деінсталювати тільки у середовищі 64-бітної Windows.
OnlyAdminCanUninstall=Ця програма може бути деінстальована тільки користувачами з адміністраторськими привілеями.
UninstallStatusLabel=Будь ласка, почекайте, поки %1 буде видалена з Вашого комп'ютера .
UninstalledAll=Програма %1 була повністю видалена з Вашого комп'ютера.
UninstalledMost=Деінсталяція %1 завершена.%n%nЧастину елементів не вдалося видалити. Ви можете видалити їй самостійно.
UninstalledAndNeedsRestart=Для завершення деінсталяції %1 необхідно виконати перезавантаження Вашого комп'ютера.%n%nВиконати перезавантаження зараз?
UninstallDataCorrupted=Файл "%1" пошкоджений. Деінсталяція неможлива

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=Видалити спільно використовуваний файл?
ConfirmDeleteSharedFile2=Система вказує, що наступний спільно використовуваний файл більше не використовується ніякими іншими застосуваннями. Підтверджуєте видалення файлу?%n%nЯкщо які-небудь програми все ще використовують цей файл, і він буде видалений, вони не зможуть працювати правильно. Якщо Ви не упевнені, виберіть «Ні». Залишений файл не нашкодить Вашій системі.
SharedFileNameLabel=Ім'я файлу:
SharedFileLocationLabel=Розміщення:
WizardUninstalling=Стан деінсталяції
StatusUninstalling=Деінсталяція %1...

; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.

[CustomMessages]

NameAndVersion=%1, версія %2
AdditionalIcons=Додаткові ярлики:
CreateDesktopIcon=Створити ярлики на &Робочому столі
CreateQuickLaunchIcon=Створити ярлики у &Панелі швидкого старту
ProgramOnTheWeb=Сайт %1 в Інтернеті
UninstallProgram=Деінсталювати %1
LaunchProgram=Запустити %1
AssocFileExtension=Зв&'язати %1 з файлами, що мають розширення  %2
AssocingFileExtension=Зв'язування %1 з файлами %2...
