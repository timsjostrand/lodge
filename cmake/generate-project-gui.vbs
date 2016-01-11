'
' Easy way to call 'generate-project.cmake' on any Windows machine with GUI.
'
' Author: Tim Sjöstrand <tim.sjostrand@gmail.com>

Option Explicit

' Select game name
Dim gameName
gameName = InputBox("Enter new project name")
If gameName = "" Then
    WScript.Quit 1
End If

' Select directory in which to create project
Dim projectDir
projectDir = SelectFolder("")
If projectDir = vbNull Then
    WScript.Quit 1
End If

' Generate full path to cmake script
Dim fso
Set fso = CreateObject("Scripting.FileSystemObject")
Dim currentDirectory
currentDirectory = fso.GetAbsolutePathName(".")
Dim cmakeScript
cmakeScript = fso.BuildPath(currentDirectory, "generate-project.cmake")
    
' Run cmake script
Dim objShell
Set objShell = WScript.CreateObject("WScript.Shell")
objShell.CurrentDirectory = projectDir
objShell.Run("cmake -DGAME_NAME=""" & gameName & """ -P """ & cmakeScript & """")
Set objShell = Nothing

Function SelectFolder(myStartFolder)
    Dim objFolder, objItem, objShell
    
    On Error Resume Next
    SelectFolder = vbNull

    Set objShell  = CreateObject("Shell.Application")
    Set objFolder = objShell.BrowseForFolder(0, "Select Folder", 0, myStartFolder)

    If IsObject(objfolder) Then SelectFolder = objFolder.Self.Path

    Set objFolder = Nothing
    Set objshell  = Nothing
    On Error Goto 0
End Function