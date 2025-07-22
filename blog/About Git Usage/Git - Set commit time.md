```powershell
Add-Content test.txt "提交于 2025-06-03"
git add test.txt
$env:GIT_AUTHOR_DATE="2025-06-03T12:00:00"
$env:GIT_COMMITTER_DATE="2025-06-03T12:00:00"
git commit -m "提交于 2025-06-03"
Remove-Item Env:GIT_AUTHOR_DATE
Remove-Item Env:GIT_COMMITTER_DATE
git push origin main

```