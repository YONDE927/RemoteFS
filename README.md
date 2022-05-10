# RemoteFS
![RemoteFS](https://user-images.githubusercontent.com/42487271/161102328-2901741a-dd1a-4254-aebb-5dcf290ffdef.png)
![operations](https://user-images.githubusercontent.com/42487271/164699265-675b5762-2bcb-48fa-b8a1-b2bd78ccde09.png)
![cache](https://user-images.githubusercontent.com/42487271/164709346-5ad5efe5-d269-4ba0-b369-730774ea1291.jpg)
![mirror-read](https://user-images.githubusercontent.com/42487271/165710680-18cf19a7-28ba-46b8-b80d-535bf7825210.png)


### メモ
mirrorの構成
インタフェース
- ミラーファイルの置き場所の設定
- ミラーファイルのサーチ
- 同期チェック
- ダウンロードタスクの実行
- ダウンロードタスクの中断

内部
- タスクキュー
- ブロックレベルのダウンロードタスク
- ミラーファイルの管理（Sqlite3)　マルチスレッド対応

構造体
- mirrorStore
    - sizesum
    - mirroconfig
    - dbsession
    - mirrored-file-list
    - permission

- mirroredFile
    - mtime
    - size
    - name
    - path


