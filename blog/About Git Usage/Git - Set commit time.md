```powershell
powershell
Add-Content test.txt "提交于 2026-07-23"
git add test.txt
$env:GIT_AUTHOR_DATE="2026-07-27T12:00:00"
$env:GIT_COMMITTER_DATE="2026-07-27T12:00:00"
git commit -m ""
Remove-Item Env:GIT_AUTHOR_DATE
Remove-Item Env:GIT_COMMITTER_DATE
git push origin main

```