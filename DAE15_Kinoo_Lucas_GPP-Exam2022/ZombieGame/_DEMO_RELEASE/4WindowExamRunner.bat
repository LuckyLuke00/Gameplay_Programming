start GPP_TEST_RELEASE.exe -x 30 -y 30 -s 0

start GPP_TEST_RELEASE.exe -x 1000 -y 30 -s 0

start GPP_TEST_RELEASE.exe -x 30 -y 540 -s 0
start GPP_TEST_RELEASE.exe -x 30 -y 540 -s 0
start GPP_TEST_RELEASE.exe -x 30 -y 540 -s 0
start GPP_TEST_RELEASE.exe -x 30 -y 540 -s 0
start GPP_TEST_RELEASE.exe -x 30 -y 540 -s 0
start GPP_TEST_RELEASE.exe -x 30 -y 540 -s 0
start GPP_TEST_RELEASE.exe -x 30 -y 540 -s 0
start GPP_TEST_RELEASE.exe -x 30 -y 540 -s 0
start GPP_TEST_RELEASE.exe -x 30 -y 540 -s 0

start GPP_TEST_RELEASE.exe -x 1000 -y 540 -s 0
echo new ActiveXObject("WScript.Shell").AppActivate("GPP_TEST_RELEASE.exe"); > tmp.js
cscript //nologo tmp.js & del tmp.js