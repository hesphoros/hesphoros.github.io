```powershell
powershell
Add-Content test.txt "提交于 2026-07-23"
git add test.txt

$env:GIT_AUTHOR_DATE="2026-10-18T12:00:00+08:00"
$env:GIT_COMMITTER_DATE="2026-10-18T12:00:00+08:00"

git commit -m "Update test.txt"

Remove-Item Env:GIT_AUTHOR_DATE
Remove-Item Env:GIT_COMMITTER_DATE

git push origin main

```

```bash
echo "提交于 2026-07-23" >> test.txt
git add test.txt
GIT_AUTHOR_DATE="2026-07-27T12:00:00" 
GIT_COMMITTER_DATE="2026-07-27T12:00:00" 
git commit -m "你的提交信息"
git push origin main
```

## 如果提交已经存在于 GitHub

如果你是想修改已经提交并推送到 GitHub 的那次提交时间，上面的命令不够，需要修改已有提交：

```bash
$env:GIT_AUTHOR_DATE="2026-10-18T12:00:00+08:00"
$env:GIT_COMMITTER_DATE="2026-10-18T12:00:00+08:00"

git commit --amend --no-edit --date="2026-10-18T12:00:00+08:00"

Remove-Item Env:GIT_AUTHOR_DATE
Remove-Item Env:GIT_COMMITTER_DATE

git push --force-with-lease origin main
```

注意： `--amend` 会改变提交哈希，强制推送可能影响其他协作者。如果只是新建一次提交，使用第一种方式即可。GitHub 显示的提交时间也可能受时区影响。