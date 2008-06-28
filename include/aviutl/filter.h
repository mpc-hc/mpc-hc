#pragma once

typedef	struct {short y, cb, cr;} PIXEL_YC;
typedef	struct {unsigned char b, g, r;} PIXEL;

//	フィルタPROC用構造体
typedef struct {
	int			flag;			//	フィルタのフラグ
	PIXEL_YC	*ycp_edit;		//	画像データへのポインタ
	PIXEL_YC	*ycp_temp;		//	テンポラリ領域へのポインタ
	int			w,h;			//	現在の画像のサイズ
	int			max_w,max_h;	//	画像領域のサイズ
	int			frame;			//	現在のフレーム番号(番号は0から)
	int			frame_n;		//	総フレーム数
	int			org_w,org_h;	//	元の画像のサイズ
	short		*audiop;		//	オーディオデータへのポインタ ( オーディオフィルタの時のみ )
								//	オーディオ形式はPCM16bitです ( 1サンプルは mono = 2byte , stereo = 4byte )
	int			audio_n;		//	オーディオサンプルの総数
	int			audio_ch;		//	オーディオチャンネル数
	PIXEL		*pixelp;		//	DIB形式のデータへのポインタ( 表示フィルタの時のみ )
	void		*editp;			//	エディットハンドル
	int			reserve[10];	//	拡張用に予約されてます
} FILTER_PROC_INFO;

#define	FILTER_PROC_INFO_FLAG_FILL_OVERAREA		1
#define	FILTER_PROC_INFO_FLAG_MASK				0x0fffffff

//	フレームステータス構造体
typedef struct {
	int		video;			//	実際の映像データ番号
	int		audio;			//	実際の音声データ番号
	int		inter;			//	フレームのインターレース
							//	FRAME_STATUS_INTER_NORMAL	: 標準
							//	FRAME_STATUS_INTER_REVERSE	: 反転
							//	FRAME_STATUS_INTER_ODD		: 奇数
							//	FRAME_STATUS_INTER_EVEN		: 偶数
							//	FRAME_STATUS_INTER_MIX		: 二重化
							//	FRAME_STATUS_INTER_AUTO		: 自動
	int		index24fps;		//	24fpの周期
	int		config;			//	フレームの設定環境の番号
	int		vcm;			//	フレームの圧縮設定の番号
	int		edit_flag;		//	編集フラグ
							//	EDIT_FRAME_EDIT_FLAG_KEYFRAME	: キーフレーム
							//	EDIT_FRAME_EDIT_FLAG_MARKFRAME	: マークフレーム
							//	EDIT_FRAME_EDIT_FLAG_DELFRAME	: 優先間引きフレーム
							//	EDIT_FRAME_EDIT_FLAG_NULLFRAME	: コピーフレーム
	int		reserve[9];		//	拡張用に予約されてます
} FRAME_STATUS;

#define	FRAME_STATUS_INTER_NORMAL		0
#define	FRAME_STATUS_INTER_REVERSE		1
#define	FRAME_STATUS_INTER_ODD			2
#define	FRAME_STATUS_INTER_EVEN			3
#define	FRAME_STATUS_INTER_MIX			4
#define	FRAME_STATUS_INTER_AUTO			5
#define	EDIT_FRAME_EDIT_FLAG_KEYFRAME		1
#define	EDIT_FRAME_EDIT_FLAG_MARKFRAME		2
#define	EDIT_FRAME_EDIT_FLAG_DELFRAME		4
#define	EDIT_FRAME_EDIT_FLAG_NULLFRAME		8

//	ファイルインフォメーション構造体
typedef struct {
	int		flag;					//	ファイルのフラグ
									//	FILE_INFO_FLAG_VIDEO	: 映像が存在する
									//	FILE_INFO_FLAG_AUDIO	: 音声が存在する
	LPSTR	name;					//	編集ファイル名
	int		w,h;					//	元のサイズ
	int		video_rate,video_scale;	//	フレームレート
	int		audio_rate;				//	音声サンプリングレート
	int		audio_ch;				//	音声チャンネル数
	int		reserve[8];				//	拡張用に予約されてます
} FILE_INFO;

#define FILE_INFO_FLAG_VIDEO	1
#define FILE_INFO_FLAG_AUDIO	2

//	システムインフォメーション構造体
typedef struct {
	int		flag;					//	システムフラグ
									//	SYS_INFO_FLAG_EDIT	: 編集中
									//	SYS_INFO_FLAG_VFAPI	: VFAPI動作時
	LPSTR	info;					//	バージョン情報
	int		filter_n;				//	登録されてるフィルタの数
	int		min_w,min_h;			//	編集出来る最小画像サイズ
	int		max_w,max_h;			//	編集出来る最大画像サイズ
	int		max_frame;				//	編集出来る最大フレーム数
	LPSTR	edit_name;				//	編集ファイル名 (ファイル名が決まっていない時は何も入っていません)
	LPSTR	project_name;			//	プロジェクトファイル名 (ファイル名が決まっていない時は何も入っていません)
	LPSTR	output_name;			//	出力ファイル名 (ファイル名が決まっていない時は何も入っていません)
	int		reserve[8];				//	拡張用に予約されてます
} SYS_INFO;

#define SYS_INFO_FLAG_EDIT	1
#define SYS_INFO_FLAG_VFAPI	2

//	外部関数構造体
typedef struct {
	void		*(*get_ycp_ofs)( void *editp,int n,int ofs );
								//	指定したフレームのAVIファイル上でのオフセット分移動した
								//	フレームの画像データのポインタを取得します
								//	データはフィルタ前のものです
								//	editp 	: エディットハンドル
								//	n	 	: フレーム番号
								//	ofs	 	: フレームからのオフセット
								//  戻り値	: 画像データへのポインタ (NULLなら失敗)
	void		*(*get_ycp)( void *editp,int n );
								//	指定したフレームの画像データのポインタを取得します
								//	データはフィルタ前のものです
								//	editp 	: エディットハンドル
								//	n	 	: フレーム番号
								//  戻り値	: 画像データへのポインタ (NULLなら失敗)
	void		*(*get_pixelp)( void *editp,int n );
								//	指定したフレームのDIB形式(RGB24bit)の画像データのポインタを取得します
								//	データはフィルタ前のものです
								//	editp 	: エディットハンドル
								//	n		: フレーム番号
								//  戻り値	: DIB形式データへのポインタ (NULLなら失敗)
	int			(*get_audio)( void *editp,int n,void *buf );
								//	指定したフレームのオーディオデータを読み込みます
								//	データはフィルタ前のものです
								//	editp* 	: エディットハンドル
								//	n		: フレーム番号
								//	buf 	: 格納するバッファ
								//  戻り値	: 読み込んだサンプル数
	BOOL		(*is_editing)( void *editp );
								//	現在編集中か調べます
								//	editp 	: エディットハンドル
								//  戻り値	: TRUEなら編集中
	BOOL		(*is_saving)( void *editp );
								//	現在保存中か調べます
								//	editp 	: エディットハンドル
								//  戻り値	: TRUEなら保存中
	int			(*get_frame)( void *editp );
								//	現在の表示フレームを取得します
								//	editp 	: エディットハンドル
								//  戻り値	: 現在のフレーム番号
	int			(*get_frame_n)( void *editp );
								//	総フレーム数を取得します
								//	editp 	: エディットハンドル
								//  戻り値	: 現在の総フレーム数
	BOOL		(*get_frame_size)( void *editp,int *w,int *h );
								//	フィルタ前のフレームのサイズを取得します
								//	editp 	: エディットハンドル
								//	w,h 	: 画像サイズの格納ポインタ
								//  戻り値	: TRUEなら成功
	int			(*set_frame)( void *editp,int n );
								//	現在の表示フレームを変更します
								//	editp 	: エディットハンドル
								//  n		: フレーム番号
								//  戻り値	: 設定されたフレーム番号
	int			(*set_frame_n)( void *editp,int n );
								//	総フレーム数を変更します
								//	editp 	: エディットハンドル
								//  n		: フレーム数
								//  戻り値	: 設定された総フレーム数
	BOOL		(*copy_frame)( void *editp,int d,int s );
								//	フレームを他のフレームにコピーします
								//	editp 	: エディットハンドル
								//	d	 	: コピー先フレーム番号
								//	s	 	: コピー元フレーム番号
								//  戻り値	: TRUEなら成功
	BOOL		(*copy_video)( void *editp,int d,int s );
								//	フレームの映像だけを他のフレームにコピーします
								//	editp 	: エディットハンドル
								//	d	 	: コピー先フレーム番号
								//	s	 	: コピー元フレーム番号
								//  戻り値	: TRUEなら成功
	BOOL		(*copy_audio)( void *editp,int d,int s );
								//	フレームの音声だけを他のフレームにコピーします
								//	editp 	: エディットハンドル
								//	d	 	: コピー先フレーム番号
								//	s	 	: コピー元フレーム番号
								//  戻り値	: TRUEなら成功
	BOOL		(*copy_clip)( HWND hwnd,void *pixelp,int w,int h );
								//	クリップボードにDIB形式(RGB24bit)の画像をコピーします
								//	hwnd 	: ウィンドウハンドル
								//	pixelp	: DIB形式データへのポインタ
								//	w,h 	: 画像サイズ
								//  戻り値	: TRUEなら成功
	BOOL		(*paste_clip)( HWND hwnd,void *editp,int n );
								//	クリップボードから画像を張りつけます
								//	hwnd 	: ウィンドウハンドル
								//	editp 	: エディットハンドル
								//  n		: フレーム番号
								//  戻り値	: TRUEなら成功
	BOOL		(*get_frame_status)( void *editp,int n,FRAME_STATUS *fsp );
								//	フレームのステータスを取得します
								//	editp 	: エディットハンドル
								//  n		: フレーム番号
								//  fps		: フレームステータスへのポインタ
								//  戻り値	: TRUEなら成功
	BOOL		(*set_frame_status)( void *editp,int n,FRAME_STATUS *fsp );
								//	フレームのステータスを変更します
								//	editp 	: エディットハンドル
								//  n		: フレーム番号
								//  fps		: フレームステータスへのポインタ
								//  戻り値	: TRUEなら成功
	BOOL		(*is_saveframe)( void *editp,int n );
								//	実際に保存されるフレームか調べます
								//	editp 	: エディットハンドル
								//  n		: フレーム番号
								//  戻り値	: TRUEなら保存されます
	BOOL		(*is_keyframe)( void *editp,int n );
								//	キーフレームかどうか調べます
								//	editp 	: エディットハンドル
								//  n		: フレーム番号
								//  戻り値	: TRUEならキーフレーム
	BOOL		(*is_recompress)( void *editp,int n );
								//	再圧縮が必要か調べます
								//	editp 	: エディットハンドル
								//  n		: フレーム番号
								//  戻り値	: TRUEなら再圧縮が必要
	BOOL		(*filter_window_update)( void *fp );
								//	設定ウィンドウのトラックバーとチェックボックスを再描画します
								//	fp	 	: フィルタ構造体のポインタ
								//  戻り値	: TRUEなら成功
	BOOL		(*is_filter_window_disp)( void *fp );
								//	設定ウィンドウが表示されているか調べます
								//	fp	 	: フィルタ構造体のポインタ
								//  戻り値	: TRUEなら表示されている
	BOOL		(*get_file_info)( void *editp,FILE_INFO *fip );
								//	編集ファイルの情報を取得します
								//	editp 	: エディットハンドル
								//  fip		: ファイルインフォメーション構造体へのポインタ
								//  戻り値	: TRUEなら成功
	LPSTR		(*get_config_name)( void *editp,int n );
								//	設定環境の名前を取得します
								//	editp 	: エディットハンドル
								//  n		: 設定環境の番号
								//  戻り値	: 設定環境の名前へのポインタ
	BOOL		(*is_filter_active)( void *fp );
								//	フィルタが有効になっているか調べます
								//	fp	 	: フィルタ構造体のポインタ
								//  戻り値	: TRUEならフィルタ有効
	BOOL		(*get_pixel_filtered)( void *editp,int n,void *pixelp,int *w,int *h );
								//	指定したフレームのDIB形式(RGB24bit)の画像データを読み込みます
								//	データはフィルタ後のものです
								//	editp 	: エディットハンドル
								//	n		: フレーム番号
								//  pixelp	: DIB形式データを格納するポインタ (NULLなら画像サイズだけを返します)
								//	w,h		: 画像のサイズ (NULLならDIB形式データだけを返します)
								//  戻り値	: TRUEなら成功
	int			(*get_audio_filtered)( void *editp,int n,void *buf );
								//	指定したフレームのオーディオデータを読み込みます
								//	データはフィルタ後のものです
								//	editp* 	: エディットハンドル
								//	n		: フレーム番号
								//	buf 	: 格納するバッファ
								//  戻り値	: 読み込んだサンプル数
	BOOL		(*get_select_frame)( void *editp,int *s,int *e );
								//	選択開始終了フレームを取得します
								//	editp 	: エディットハンドル
								//	s		: 選択開始フレーム
								//	e		: 選択終了フレーム
								//  戻り値	: TRUEなら成功
	BOOL		(*set_select_frame)( void *editp,int s,int e );
								//	選択開始終了フレームを設定します
								//	editp 	: エディットハンドル
								//	s		: 選択開始フレーム
								//	e		: 選択終了フレーム
								//  戻り値	: TRUEなら成功
	BOOL		(*rgb2yc)( PIXEL_YC *ycp,PIXEL *pixelp,int w );
								//	PIXELからPIXEL_YCに変換します
								//	ycp		: PIXEL_YC構造体へのポインタ
								//	pixelp 	: PIXEL構造体へのポインタ
								//	w		: 構造体の数
								//  戻り値	: TRUEなら成功
	BOOL		(*yc2rgb)( PIXEL *pixelp,PIXEL_YC *ycp,int w );
								//	PIXEL_YCからPIXELに変換します
								//	pixelp 	: PIXEL構造体へのポインタ
								//	ycp		: PIXEL_YC構造体へのポインタ
								//	w		: 構造体の数
								//  戻り値	: TRUEなら成功
	BOOL		(*dlg_get_load_name)( LPSTR name,LPSTR filter,LPSTR def );
								//	ファイルダイアログを使って読み込むファイル名を取得します
								//	name	: ファイル名を格納するポインタ
								//	filter	: ファイルフィルタ
								//  def		: デフォルトのファイル名
								//  戻り値	: TRUEなら成功 FALSEならキャンセル
	BOOL		(*dlg_get_save_name)( LPSTR name,LPSTR filter,LPSTR def );
								//	ファイルダイアログを使って書き込むファイル名を取得します
								//	name	: ファイル名を格納するポインタ
								//	filter	: ファイルフィルタ
								//  def		: デフォルトのファイル名
								//  戻り値	: TRUEなら成功 FALSEならキャンセル
	int			(*ini_load_int)( void *fp,LPSTR key,int n );
								//	INIファイルから数値を読み込む
								//	fp	 	: フィルタ構造体のポインタ
								//	key		: アクセス用のキーの名前
								//  n		: デフォルトの数値
								//  戻り値	: 読み込んだ数値
	int			(*ini_save_int)( void *fp,LPSTR key,int n );
								//	INIファイルに数値を書き込む
								//	fp	 	: フィルタ構造体のポインタ
								//	key		: アクセス用のキーの名前
								//  n		: 書き込む数値
								//  戻り値	: 書き込んだ数値
	BOOL		(*ini_load_str)( void *fp,LPSTR key,LPSTR str,LPSTR def );
								//	INIファイルから文字列を読み込む
								//	fp	 	: フィルタ構造体のポインタ
								//	key		: アクセス用のキーの名前
								//  str		: 文字列を読み込むバッファ
								//  def		: デフォルトの文字列
								//  戻り値	: TRUEなら成功
	BOOL		(*ini_save_str)( void *fp,LPSTR key,LPSTR str );
								//	INIファイルに文字列を書き込む
								//	fp	 	: フィルタ構造体のポインタ
								//	key		: アクセス用のキーの名前
								//  n		: 書き込む文字列
								//  戻り値	: TRUEなら成功
	BOOL		(*get_source_file_info)( void *editp,FILE_INFO *fip,int source_file_id );
								//	指定したファイルIDのファイルの情報を取得します
								//	editp 	: エディットハンドル
								//  fip		: ファイルインフォメーション構造体へのポインタ
								//	souce_file_id
								//			: ファイルID
								//  戻り値	: TRUEなら成功
	BOOL		(*get_source_video_number)( void *editp,int n,int *source_file_id,int *source_video_number );
								//	指定したフレームのソースのファイルIDとフレーム番号を取得します
								//	editp 	: エディットハンドル
								//	n		: フレーム番号
								//	souce_file_id
								//			: ファイルIDを格納するポインタ
								//	souce_video_number
								//			: フレーム番号を格納するポインタ
								//  戻り値	: TRUEなら成功
	BOOL		(*get_sys_info)( void *editp,SYS_INFO *sip );
								//	システムの情報を取得します
								//	editp 	: エディットハンドル
								//  sip		: システムインフォメーション構造体へのポインタ
								//  戻り値	: TRUEなら成功
	void 		*(*get_filterp)( int filter_id );
								//	指定のフィルタIDのフィルタ構造体へのポインタを取得します
								//	filter_id
								//		 	: フィルタID (0～登録されてるフィルタの数-1までの値)
								//  戻り値	: フィルタ構造体へのポインタ (NULLなら失敗)
	int			reserve[6];
} EXFUNC;

//	フィルタ構造体
typedef struct {
	int		flag;				//	フィルタのフラグ
								//	FILTER_FLAG_ALWAYS_ACTIVE		: フィルタを常にアクティブにします
								//	FILTER_FLAG_CONFIG_POPUP		: 設定をポップアップメニューにします
								//	FILTER_FLAG_CONFIG_CHECK		: 設定をチェックボックスメニューにします
								//	FILTER_FLAG_CONFIG_RADIO		: 設定をラジオボタンメニューにします
								//	FILTER_FLAG_EX_DATA				: 拡張データを保存出来るようにします
								//	FILTER_FLAG_PRIORITY_HIGHEST	: フィルタのプライオリティを常に最上位にします
								//	FILTER_FLAG_PRIORITY_LOWEST		: フィルタのプライオリティを常に最下位にします
								//	FILTER_FLAG_WINDOW_THICKFRAME	: サイズ変更可能なウィンドウを作ります
								//	FILTER_FLAG_WINDOW_SIZE			: 設定ウィンドウのサイズを指定出来るようにします
								//	FILTER_FLAG_DISP_FILTER			: 表示フィルタにします
								//	FILTER_FLAG_REDRAW				: 再描画をplugin側で処理するようにします
								//	FILTER_FLAG_EX_INFORMATION		: フィルタの拡張情報を設定できるようにします
								//	FILTER_FLAG_INFORMATION			: FILTER_FLAG_EX_INFORMATION を使うようにして下さい
								//	FILTER_FLAG_NO_CONFIG			: 設定ウィンドウを表示しないようにします
								//	FILTER_FLAG_AUDIO_FILTER		: オーディオフィルタにします
								//	FILTER_FLAG_RADIO_BUTTON		: チェックボックスをラジオボタンにします
								//	FILTER_FLAG_WINDOW_HSCROLL		: 水平スクロールバーを持つウィンドウを作ります
								//	FILTER_FLAG_WINDOW_VSCROLL		: 垂直スクロールバーを持つウィンドウを作ります
								//	FILTER_FLAG_IMPORT				: インポートメニューを作ります
								//	FILTER_FLAG_EXPORT				: エクスポートメニューを作ります
	int		x,y;				//	設定ウインドウのサイズ (FILTER_FLAG_WINDOW_SIZEが立っている時に有効)
	TCHAR	*name;				//	フィルタの名前
	int		track_n;			//	トラックバーの数
	TCHAR	**track_name;		//	トラックバーの名前郡へのポインタ(トラックバー数が0ならNULLでよい)
	int		*track_default;		//	トラックバーの初期値郡へのポインタ(トラックバー数が0ならNULLでよい)
	int		*track_s,*track_e;	//	トラックバーの数値の下限上限 (NULLなら全て0～256)
	int		check_n;			//	チェックボックスの数
	TCHAR	**check_name;		//	チェックボックスの名前郡へのポインタ(チェックボックス数が0ならNULLでよい)
	int		*check_default;		//	チェックボックスの初期値郡へのポインタ(チェックボックス数が0ならNULLでよい)
	BOOL	(*func_proc)( void *fp,FILTER_PROC_INFO *fpip );
								//	フィルタ処理関数へのポインタ (NULLなら呼ばれません)
	BOOL	(*func_init)( void *fp );
								//	開始時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	BOOL	(*func_exit)( void *fp );
								//	終了時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	BOOL	(*func_update)( void *fp );
								//	設定が変更されたときに呼ばれる関数へのポインタ (NULLなら呼ばれません)
	BOOL 	(*func_WndProc)( HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam,void *editp,void *fp );
								//	設定ウィンドウにウィンドウメッセージが来た時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
								//	通常のメッセージ以外に以下の拡張メッセージが送られます
								//	WM_FILTER_UPDATE		: フィルタ設定や編集内容が変更された直後に送られます
								//	WM_FILTER_FILE_OPEN		: 編集ファイルがオープンされた直後に送られます
								//	WM_FILTER_FILE_CLOSE	: 編集ファイルがクローズされる直前に送られます
								//	WM_FILTER_INIT			: 開始直後に送られます
								//	WM_FILTER_EXIT			: 終了直前に送られます
								//	WM_FILTER_SAVE_START	: セーブが開始される直前に送られます
								//	WM_FILTER_SAVE_END		: セーブが終了された直後に送られます
								//	WM_FILTER_IMPORT		: インポートが選択された直後に送られます
								//	WM_FILTER_EXPORT		: エクスポートが選択された直後に送られます
								//	戻り値をTRUEにすると編集内容が更新されたとして全体が再描画されます
	int		*track,*check;		//	システムで使いますので使用しないでください
	void	*ex_data_ptr;		//	拡張データ領域へのポインタ (FILTER_FLAG_EX_DATAが立っている時に有効)
	int		ex_data_size;		//	拡張データサイズ (FILTER_FLAG_EX_DATAが立っている時に有効)
	TCHAR	*information;		//	フィルタ情報へのポインタ (FILTER_FLAG_EX_INFORMATIONが立っている時に有効)
	BOOL	(*func_save_start)( void *fp,int s,int e,void *editp );
								//	セーブが開始される直前に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	BOOL	(*func_save_end)( void *fp,void *editp );
								//	セーブが終了した直前に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	EXFUNC	*exfunc;			//	外部関数テーブルへのポインタ
	HWND	hwnd;				//	ウィンドウハンドル
	HINSTANCE	dll_hinst;		//	DLLのインスタンスハンドル
	int		reserve[8];			//	拡張用に予約されてます
} FILTER;
#define	FILTER_FLAG_ACTIVE				1
#define	FILTER_FLAG_ALWAYS_ACTIVE		4
#define	FILTER_FLAG_CONFIG_POPUP		8
#define	FILTER_FLAG_CONFIG_CHECK		16
#define	FILTER_FLAG_CONFIG_RADIO		32
#define	FILTER_FLAG_EX_DATA				1024
#define	FILTER_FLAG_PRIORITY_HIGHEST	2048
#define	FILTER_FLAG_PRIORITY_LOWEST		4096
#define	FILTER_FLAG_WINDOW_THICKFRAME	8192
#define	FILTER_FLAG_WINDOW_SIZE			16384
#define	FILTER_FLAG_DISP_FILTER			32768
#define	FILTER_FLAG_REDRAW				0x20000
#define	FILTER_FLAG_EX_INFORMATION		0x40000
#define	FILTER_FLAG_INFORMATION			0x80000
#define	FILTER_FLAG_NO_CONFIG			0x100000
#define	FILTER_FLAG_AUDIO_FILTER		0x200000
#define	FILTER_FLAG_RADIO_BUTTON		0x400000
#define	FILTER_FLAG_WINDOW_HSCROLL		0x800000
#define	FILTER_FLAG_WINDOW_VSCROLL		0x1000000
#define	FILTER_FLAG_IMPORT				0x10000000
#define	FILTER_FLAG_EXPORT				0x20000000
#define WM_FILTER_UPDATE				(WM_USER+100)
#define WM_FILTER_FILE_OPEN				(WM_USER+101)
#define WM_FILTER_FILE_CLOSE			(WM_USER+102)
#define WM_FILTER_INIT					(WM_USER+103)
#define WM_FILTER_EXIT					(WM_USER+104)
#define WM_FILTER_SAVE_START			(WM_USER+105)
#define WM_FILTER_SAVE_END				(WM_USER+106)
#define WM_FILTER_IMPORT				(WM_USER+107)
#define WM_FILTER_EXPORT				(WM_USER+108)

//	フィルタDLL用構造体
typedef struct {
	int		flag;
	int		x,y;
	TCHAR	*name;
	int		track_n;
	TCHAR	**track_name;
	int		*track_default;
	int		*track_s,*track_e;
	int		check_n;
	TCHAR	**check_name;
	int		*check_default;
	BOOL	(*func_proc)( FILTER *fp,FILTER_PROC_INFO *fpip );
	BOOL	(*func_init)( FILTER *fp );
	BOOL	(*func_exit)( FILTER *fp );
	BOOL	(*func_update)( FILTER *fp );
	BOOL 	(*func_WndProc)( HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam,void *editp,FILTER *fp );
	int		*track,*check;
	void	*ex_data_ptr;
	int		ex_data_size;
	TCHAR	*information;
	BOOL	(*func_save_start)( void *fp,int s,int e,void *editp );
	BOOL	(*func_save_end)( void *fp,void *editp );
	EXFUNC	*exfunc;
	HWND	hwnd;
	HINSTANCE	dll_hinst;
	int		reserve[8];
} FILTER_DLL;

#define	MID_FILTER_BUTTON			12004

BOOL func_proc( FILTER *fp,FILTER_PROC_INFO *fpip );
BOOL func_init( FILTER *fp );
BOOL func_exit( FILTER *fp );
BOOL func_update( FILTER *fp );
BOOL func_WndProc( HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam,void *editp,FILTER *fp );
BOOL func_save_start( FILTER *fp,int s,int e,void *editp );
BOOL func_save_end( FILTER *fp,void *editp );


