
//______________________________________________________________________________
void edit(char *file)
{
   char s[64], *e;
   if (!strcmp(gSystem->GetName(), "WinNT")) {
      if (e = getenv("EDITOR"))
         sprintf(s, "start %s %s", e, file);
      else
         sprintf(s, "start notepad %s", file);
   } else {
      if (e = getenv("EDITOR"))
         sprintf(s, "%s %s", e, file);
      else
         sprintf(s, "xterm -e vi %s &", file);
   }
   gSystem.Exec(s);
}

//______________________________________________________________________________
void ls(char *path=0)
{
   char s[256] = (!strcmp(gSystem->GetName(), "WinNT")) ? "dir /w " : "ls ";
   if (path) strcat(s,path);
   gSystem.Exec(s);
}

//______________________________________________________________________________
void dir(char *path=0)
{
   char s[256] = (!strcmp(gSystem->GetName(), "WinNT")) ? "dir " : "ls -l ";
   if (path) strcat(s,path);
   gSystem.Exec(s);
}

//______________________________________________________________________________
char *pwd()
{
    return gSystem.WorkingDirectory();
}

//______________________________________________________________________________
char *cd(char *path=0)
{
 if (path)
   gSystem.ChangeDirectory(path);
 return pwd();
}

//______________________________________________________________________________
void bexec(char *macro)
{
   if (gROOT->IsBatch()) printf("Processing benchmark: %s\n",macro);
   TPaveText *summary = (TPaveText*)bench->GetPrimitive("TPave");
   TText *tmacro = summary->GetLineWith(macro);
   if (tmacro) tmacro->SetTextColor(4);
   bench->Modified(); bench->Update();

   gROOT->Macro(macro);

   TPaveText *summary2 = (TPaveText*)bench->GetPrimitive("TPave");
   TText *tmacro2 = summary2->GetLineWith(macro);
   if (tmacro2) tmacro2->SetTextColor(2);
   bench->Modified(); bench->Update();
}
