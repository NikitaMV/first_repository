#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string.h>
#include "vix.h"

using namespace std;

#define  CONNTYPE    VIX_SERVICEPROVIDER_VMWARE_WORKSTATION
#define  HOSTNAME ""
#define  HOSTPORT 0
#define  USERNAME ""
#define  PASSWORD ""
#define  VMPOWEROPTIONS  0


int main(int argc, char** argv)
{
	VixError err;
	int isLoggedIn = 0;	
	VixHandle hostHandle = VIX_INVALID_HANDLE;
	VixHandle vmHandle = VIX_INVALID_HANDLE;	
	VixHandle jobHandle = VIX_INVALID_HANDLE;

	if (argc != 11)
	{
		cout << "Number of arguments is incorrect" << endl;
		return 0;
	}
		
	char* browser = argv[2];
	char* version = argv[4];
	char* vmxPath = argv[6];
	char* login = argv[8];
	char* password = argv[10];
	char* silent_install_options = "";
	char path_to_browser_on_host[150] = "c:\\Browsers\\";
	char* path_to_browser_on_virual_machine;	
	
	char buf_browser[20], buf_version[20];
	strcpy(buf_browser, browser);
	strcpy(buf_version, version);

	strcat(path_to_browser_on_host, buf_browser);
	strcat(path_to_browser_on_host, "_");
	strcat(path_to_browser_on_host, buf_version);
	strcat(path_to_browser_on_host, ".exe");		

	path_to_browser_on_virual_machine = path_to_browser_on_host;


	if (!strcmp(browser, "opera"))
		silent_install_options = "/silent";	
	else if (!strcmp(browser, "firefox"))
		silent_install_options = "-ms";
	else if (!strcmp(browser, "chrome"))	
		silent_install_options = "/silent /install";


	jobHandle = VixHost_Connect(VIX_API_VERSION, CONNTYPE, HOSTNAME, HOSTPORT, USERNAME, PASSWORD, 0, VIX_INVALID_HANDLE, NULL, NULL);
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_JOB_RESULT_HANDLE, &hostHandle, VIX_PROPERTY_NONE);
	if (VIX_FAILED(err)) {
		cout << "Cannot connect to host" << endl;
		goto abort;
	}
	Vix_ReleaseHandle(jobHandle);
	cout << "Connect to host" << endl;

	
	jobHandle = VixVM_Open(hostHandle, vmxPath, NULL, NULL); 
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_JOB_RESULT_HANDLE, &vmHandle, VIX_PROPERTY_NONE); 
	if (VIX_FAILED(err)) {
		cout << "Cannot find this virtual machine" << endl;
		goto abort;
	}
	Vix_ReleaseHandle(jobHandle);
	cout << "Open virtual machine" << endl;


	jobHandle = VixVM_PowerOn(vmHandle, VMPOWEROPTIONS, VIX_INVALID_HANDLE, NULL, NULL); 
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_FAILED(err)) {
		cout << "Cannot power on this virtual machine" << endl;
		goto abort;		
	}
	Vix_ReleaseHandle(jobHandle);
	cout << "Power on virtual machine" << endl;


	jobHandle = VixVM_WaitForToolsInGuest(vmHandle, 300, NULL, NULL); 
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_OK != err) {
		cout << "VMware tools are not installed in this virtual machine" << endl;
		goto abort;
	}
	Vix_ReleaseHandle(jobHandle);


	jobHandle = VixVM_LoginInGuest(vmHandle, login, password, 0, NULL, NULL); 
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_OK != err) {
		cout << "Incorrect login or password" << endl;
		goto abort;		
	}
	isLoggedIn = 1;
	cout << "Login" << endl;
	Vix_ReleaseHandle(jobHandle);


	jobHandle = VixVM_CreateDirectoryInGuest(vmHandle, "c:\\Browsers", VIX_INVALID_HANDLE, NULL, NULL);
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_OK != err) {
		cout << "Cannot create folder in this virtual machine. Maybe this folder exists" << endl;
		goto abort;
	}
	Vix_ReleaseHandle(jobHandle);


	jobHandle = VixVM_CopyFileFromHostToGuest(vmHandle,	path_to_browser_on_host, path_to_browser_on_virual_machine, 0, VIX_INVALID_HANDLE, NULL, NULL); 	
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_OK != err) {
		cout << "Copy failed" << endl;
		goto abort;
	}
	Sleep(30000);
	cout << "Copied" << endl;
	Vix_ReleaseHandle(jobHandle);
	

	jobHandle = VixVM_RunProgramInGuest(vmHandle, path_to_browser_on_virual_machine, silent_install_options, 0, VIX_INVALID_HANDLE, NULL, NULL);
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);	
	if (VIX_OK != err) {	
		cout << "Program cannot run" << endl;
		goto abort;
	}
	Sleep(30000);
	cout << "Browser has been successfully installed" << endl;
	Vix_ReleaseHandle(jobHandle);


	jobHandle = VixVM_PowerOff(vmHandle, VIX_VMPOWEROP_NORMAL, NULL, NULL);
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_FAILED(err)) 
	{
		cout << "Cannot power off this virtual machine" << endl;
		goto abort;
	}
	cout << "Power off" << endl;


	jobHandle = VixVM_PowerOn(vmHandle, VIX_VMPOWEROP_LAUNCH_GUI, VIX_INVALID_HANDLE, NULL, NULL);
	err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
	if (VIX_FAILED(err)) {
		cout << "Cannot power on this virtual machine" << endl;
		goto abort;
	}
	Vix_ReleaseHandle(jobHandle);
	cout << "You can use new browser" << endl;


abort:

	if (isLoggedIn) {		
		Vix_ReleaseHandle(jobHandle);		
		jobHandle = VixVM_LogoutFromGuest(vmHandle,	NULL, NULL);
		err = VixJob_Wait(jobHandle, VIX_PROPERTY_NONE);
		if (VIX_OK != err) {			
		}
	}

	Vix_ReleaseHandle(jobHandle);
	Vix_ReleaseHandle(vmHandle);
	VixHost_Disconnect(hostHandle);

	return 0;
}