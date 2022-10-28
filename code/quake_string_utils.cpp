static void RTrim(char *String)
{
    size_t Length = strlen(String);
    while (--Length)
    {
        if (String[Length] == ' ' ||
            String[Length] == '\n' ||
            String[Length] == '\t' ||
            String[Length] == '\v' ||
            String[Length] == '\f' ||
            String[Length] == '\r')
        {
            String[Length] = '\0';
        }
        else
        {
            break;
        }
    }
}