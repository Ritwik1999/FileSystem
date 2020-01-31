// This is a basic file system with a smooth code flow, does have basic file operations, but is only bound for a single run

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct docs
{
    FILE *address;
    char file_name[30];
    long int size;
    int blocks;
    char owner[6];
    int permission;
};

int perm_num = 1; // to represent who is logged on.

struct docs doc[10];

int permission[8][3] = {{0, 0, 0},
                        {0, 0, 1},
                        {0, 1, 0},
                        {0, 1, 1},
                        {1, 0, 0},
                        {1, 0, 1},
                        {1, 1, 0},
                        {1, 1, 1}};

void modify_file(char *, int);

void login()
{
    char name[10], password[10];
    int auth;
    printf("\nHit 1 to login or 2 for a guest session: \n");
    scanf("%d", &auth);
    switch (auth)
    {
    case 1:
        printf("ID: ");
        scanf("%s", name);
        printf("Password: ");
        scanf("%s", password);
        if (!((strcmp(name, "user") == 0 || strcmp(name, "group") == 0) && (strcmp(password, "123") == 0)))
        {
            printf("Invalid login details");
            exit(0);
        }
        else
        {
            printf("Login Successful");
            if (strcmp(name, "group") == 0)
            {
                perm_num = 2;
            }
            else
            {
                perm_num = 1;
            }
        }
        break;

    case 2:
        printf("\nGuest Session\n____________________________");
        perm_num = 3;
        break;

    default:
        printf("\nInvalid");
        exit(0);
        break;
    }
    return;
}

int validate(int num)
{
    if (num == 0)
    {
        return 0;
    }
    int temp = num, len = 0, flag = 1;
    while (num > 0)
    {
        if (num % 10 > 8)
        {
            flag = 0;
            break;
        }
        num = num / 10;
        len++;
    }
    if (len == 3 && flag == 1)
    {
        return 0;
    }
    return -1;
}

//returns row number for permission matrix
int pos_of_perm(int loc)
{
    int digit;
    if (perm_num == 1)
    {
        digit = (doc[loc].permission) / 100;
    }
    else
    {
        if (perm_num == 2)
        {
            digit = ((doc[loc].permission) / 10) % 10;
        }
        else
        {
            digit = (doc[loc].permission) % 10;
        }
    }
    return digit;
}

int used_blocks()
{
    int i = 0, res = 0;
    while (doc[i].address != NULL)
    {
        res += doc[i].blocks;
        i++;
    }
    return res;
}

void trimLeading(char *str)
{
    int index = 0, i;
    while (str[index] == ' ' || str[index] == '\t' || str[index] == '\n')
    {
        index++;
    }
    if (index != 0)
    {
        i = 0;
        while (str[i + index] != '\0')
        {
            str[i] = str[i + index];
            i++;
        }
        str[i] = '\0';
    }
}

int file_number(char *name)
{
    int index = -1, i = 0;
    while (strcmp(doc[i].file_name, "") != 0)
    {
        if (strcmp(doc[i].file_name, name) == 0)
        {
            index = i;
            break;
        }
        i += 1;
    }
    free(name);
    return index;
}

void create_file()
{
    if (perm_num == 3)
    {
        printf("\nYou cannot create a file in guest session.\n");
        return;
    }
    char name[30];
    int choice;
    printf("Enter the name of the file to be created along with its extension:");
    scanf("%s", name);
    int loc = 0;
    if (file_number(name) == -1)
    {
        while (doc[loc].address != NULL)
            loc += 1;

        if (used_blocks() <= 400)
        {
            FILE *fp = fopen(name, "w");
            if (fp != NULL)
            {
                printf("\nFile created successfully.");
                doc[loc].address = fp;
                strcpy(doc[loc].owner, "user");
                doc[loc].permission = 764;
                strcpy(doc[loc].file_name, name);
                printf("\nWould you like to add text to the file right away(Enter 1 to answer in the affirmative): ");
                scanf("%d", &choice);
                if (choice == 1)
                {
                    fclose(fp);
                    modify_file(name, loc);
                }
            }
            else
            {
                printf("Could not create file.");
            }
            fclose(fp);
        }
        else
        {
            printf("Memory Insufficient, for file creation.");
            return;
        }
    }
    else
    {
        printf("File with that name already exists. Procedure called once again\n");
        create_file();
    }
}

void read_file()
{
    char name[30];
    char ch;
    printf("Enter the complete name of the file you want to read.");
    scanf("%s", name);
    int loc = file_number(name);
    if (loc == -1)
    {
        int option;
        printf("File does not exist. Would you like to create it now(Enter 1 to answer in the affirmative): ");
        scanf("%d", &option);
        if (option == 1)
        {
            create_file();
        }
        else
        {
            return;
        }
    }
    else
    {
        int row = pos_of_perm(loc);
        if (permission[row][0] == 0)
        {
            printf("You dont have the permission to read this file.\n");
            return;
        }
        else
        {
            FILE *fp = fopen(name, "r");
            ch = fgetc(fp);
            while (ch != EOF)
            {
                printf("%c", ch);
                ch = fgetc(fp);
            }
            fclose(fp);
            return;
        }
    }
}

void modify_file(char *name, int loc)
{
    if (name != NULL && loc != -1)
    {
        int row = pos_of_perm(loc);
        if (permission[row][2] == 0)
        {
            printf("You don't have the permission to modify this file.\n");
            return;
        }
        FILE *fp = fopen(name, "w");
        long int size = 0;
        int blocks = 0, i = 0;
        char *string = (char *)malloc(409600);
        printf("Enter the text to be inserted.(Tab, then return to stop reading): ");
        scanf("%[^\t]s", string);
        trimLeading(string);
        if (used_blocks() * 1024 + strlen(string) >= 409600)
        {
            printf("\nInsufficient memory to insert the given string.");
            return;
        }
        fputs(string, fp);
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        doc[loc].size = size;
        while (size > 0)
        {
            blocks++;
            size -= 1024;
        }
        doc[loc].blocks = blocks;
        fclose(fp);
        free(string);
        printf("\nText inserted successfully.");
    }
    else
    {
        char name[30];
        printf("Enter the full name of the file to be modified.");
        scanf("%s", name);
        loc = file_number(name);
        if (loc == -1)
        {
            printf("No such file exists.");
            return;
        }
        else
        {
            int row = pos_of_perm(loc);
            if (permission[row][2] == 0)
            {
                printf("You don't have the permission to modify this file\n");
                return;
            }
            int choice = 1;
            printf("Enter 1 for overwriting the file or 2 for appending into it: ");
            scanf("%d", &choice);
            int blocks = 0, i = 0;
            FILE *fp;
            if (choice == 1)
            {
                fp = fopen(name, "w");
            }
            else
            {
                fp = fopen(name, "a");
            }
            char *string = (char *)malloc(409600);
            printf("Enter the text to be inserted.(Tab, then return to stop reading):");
            scanf("%[^\t]s", string);
            fseek(fp, 0, SEEK_SET);
            trimLeading(string);
            if (used_blocks() * 1024 + strlen(string) >= 409600)
            {
                printf("\nInsufficient memory to insert the given string.");
                return;
            }
            fputs(string, fp);
            printf("Modification Successful\n");
            fseek(fp, 0, SEEK_END);
            long int size = ftell(fp);
            doc[loc].size = size;
            while (size > 0)
            {
                blocks++;
                size -= 1024;
            }
            doc[loc].blocks = blocks;
            fclose(fp);
        }
    }
}

void rename_file()
{
    char name[30], new_name[30];
    printf("Enter the complete name of the file you want to rename: ");
    scanf("%s", name);
    int loc = file_number(name);
    if (loc != -1)
    {
        int row = pos_of_perm(loc);
        if (permission[row][2] == 0)
        {
            printf("You don't have the permission to rename this file\n");
            return;
        }
        printf("Enter the new name of the file: ");
        scanf("%s", new_name);
        int i = 0;
        while (doc[i].address != NULL)
        {
            if (strcmp(doc[i].file_name, new_name) == 0)
            {
                printf("\nFile with this name already exists. Returning to menu....");
                return;
            }
            i++;
        }
        strcpy(doc[loc].file_name, new_name);
        int res = rename(name, new_name);
        if (res == 0)
        {
            printf("\nRename Successful");
        }
        else
        {
            printf("\nSorry, the file could not be renamed.");
        }
        return;
    }
    else
    {
        printf("No file with the given name was found (Did you enter the name along with it's extension?)");
        return;
    }
}

void delete_file()
{
    char name[30];
    printf("\nEnter the complete name of the file you want to delete: ");
    scanf("%s", name);
    int loc = file_number(name);
    if (loc != -1)
    {
        int row = pos_of_perm(loc);
        if (permission[row][2] == 0)
        {
            printf("You don't have the permission to delete this file\n");
            return;
        }
        int status = remove(name);
        if (status == 0)
        {
            while (strcmp(doc[loc].file_name, "") != 0)
            {
                doc[loc].address = doc[loc + 1].address;
                strcpy(doc[loc].file_name, doc[loc + 1].file_name);
                doc[loc].size = doc[loc + 1].size;
                doc[loc].blocks = doc[loc + 1].blocks;
                loc++;
            }
            printf("\nFile Deleted Successfully");
        }
        else
        {
            printf("\nSorry, some error occurred. Could not delete file.");
            return;
        }
    }
    else
    {
        printf("No file with the given name was found (Did you enter the name along with it's extension?)");
        return;
    }
}

void view_FAT()
{
    int limit = 0;
    while (strcmp(doc[limit].file_name, "") != 0)
    {
        limit++;
    }
    printf("--------------------------------------------------------------------------------------------------------------\n");
    printf("File_no\tFile_name\tFile_Address\tFile_Size\tNumber_of_blocks\tOwnership\tPermissions\n");
    printf("______________________________________________________________________________________________________________\n");
    for (int i = 0; i < limit; i++)
    {
        printf("  %d\t%s\t%p\t\t%ld\t\t%d\t\t%s\t\t%d\n", i + 1, doc[i].file_name, doc[i].address, doc[i].size, doc[i].blocks, doc[i].owner, doc[i].permission);
    }
    printf("\n--------------------------------------------------------------------------------------------------------------\n");
}

void file_owner()
{
    char name[30];
    printf("Enter the name of the file, for which ownership is to be changed: ");
    scanf("%s", name);
    int loc = file_number(name);
    if (loc != -1)
    {
        printf("The current owner is: %s\n", doc[loc].owner);
        printf("Enter the new owner (Options: user, group, all) :- ");
        scanf("%s", name);
        int i = 0;
        while (name[i] != '\0')
        {
            name[i] = tolower(name[i]);
            i++;
        }
        if (strcmp(name, "user") == 0 || strcmp(name, "group") == 0 || strcmp(name, "all") == 0)
        {
            strcpy(doc[loc].owner, name);
            printf("Ownership changed.");
            return;
        }
        else
        {
            printf("No such owner value defined.");
            return;
        }
    }
    else
    {
        printf("File not found.");
        return;
    }
}

void file_rights()
{
    printf("\nA file has 3 types of permissions: read(r), write(w), and modify(m)\n");
    printf("We represent permissions for user, group, and all in the same order, folowing a absolute(numeric) mode of representation:\n");
    printf("------------------------------------------------\n");
    printf("Number      Permission Type             Symbol\n");
    printf("------------------------------------------------\n");

    printf("0           No Permission               ---\n");
    printf("1           Modify                      --m\n");
    printf("2           Write                       -w-\n");
    printf("3           Modify + Write              -wm\n");
    printf("4           Read                        r--\n");
    printf("5           Read + Modify               r-m\n");
    printf("6           Read + Write                rw-\n");
    printf("7           Read + Write + Modify       rwm\n");
    printf("------------------------------------------------\n");
    printf("The permission granted for a newly created file, is 764, which represents rwmrw-r--\n");

    char name[30];
    printf("\nEnter the full name of the file whose permissions are to be modified: ");
    scanf("%s", name);
    int loc = file_number(name);
    if (loc != -1)
    {
        int new_per;
        printf("\nEnter the new permissions for the file, in absolute(numeric) representation: ");
        scanf("%d", &new_per);
        if (validate(new_per) == 0)
        {
            doc[loc].permission = new_per;
            printf("Permissions changed");
            return;
        }
        else
        {
            printf("Invalid permission given.");
            return;
        }
    }
    else
    {
        printf("\nFile not found.");
        return;
    }

    return;
}

int main()
{
    for (int i = 0; i < 30; i++)
    {
        doc[i].address = NULL;
        doc[i].blocks = 0;
        doc[i].size = 0;
        strcpy(doc[i].file_name, "");
    }
    int run = 1;
    printf("**************WELCOME**************\n");
    printf("This File System assumes a storage space of about 400KB, and is paritioned into 400 blocks of 1KB each.");
    login();
    printf("\n\nOperations\n___________________________________________________\n");
    int choice;
    while (run == 1)
    {
        printf("\nHit 1 to create a new file");
        printf("\nHit 2 to read (an existing) file");
        printf("\nHit 3 to modify (an existing) file");
        printf("\nHit 4 to rename an existing file");
        printf("\nHit 5 to delete an existing file");
        printf("\nHit 6 to enter priviledged mode");
        printf("\nHit 7 to login with different credentials.");
        printf("\nHit 8 to exit\n");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            create_file();
            break;

        case 2:
            read_file();
            break;

        case 3:
            modify_file(NULL, -1);
            break;

        case 4:
            rename_file();
            break;

        case 5:
            delete_file();
            break;

        case 6:
            printf("**************PRIVILEGED MODE**************\n");
            char global_user[30], global_password[30];
            printf("Enter the global user credentials");
            printf("\nUsername: ");
            scanf("%s", global_user);
            printf("Password: ");
            scanf("%s", global_password);
            if (strcmp(global_user, "global_user") == 0 && strcmp(global_password, "12345") == 0)
            {
                int choice;
                printf("Hello, Global user. You have the privilege to access the FAT Table, and modify file permissions and change file ownership.");
                while (choice != 4)
                {
                    printf("\nHit 1 to view the FAT Table, 2 for changing file ownership, 3 to modify file permissions, or 4 to exit: ");
                    scanf("%d", &choice);
                    switch (choice)
                    {
                    case 1:
                        view_FAT();
                        break;

                    case 2:
                        file_owner();
                        break;

                    case 3:
                        file_rights();
                        break;

                    case 4:
                        break;

                    default:
                        printf("Invalid option.");
                        break;
                    }
                }
            }
            else
            {
                printf("Invalid Global user credentials.");
                break;
            }
            break;

        case 7:
            login();
            break;

        case 8:
            printf("**************THANK YOU**************\n");
            run = 2;
            break;

        default:
            printf("Invalid Option");
            break;
        }
    }
    return 0;
}