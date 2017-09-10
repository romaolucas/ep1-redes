#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    char from[100];
    char to[100];
    char cc[100];
    char bcc[100];
    char subject[100];
    char date[100];
    char message_id[100];
    char priority[100];
    char x_priority[100];
    char references[100];
    char newsgroup[100];
    char in_reply_to[100];
    char content_type[100];
    char reply_to[100];
    char read_part[50];
    char aux_from[100];
    int read_content_type = 0;
    FILE *email;
    email = fopen(argv[1], "r");
    while (fscanf(email, "%s", read_part) != EOF) {
        if (strcmp(read_part, "From:") == 0) {
            printf("read_part: %s\n", read_part);
            strcpy(aux_from, read_part);
            printf("aux from agora: %s\n", aux_from);
            fgets(aux_from + strlen(read_part), 100 - strlen(read_part), email);
            printf("aux from depois: %s\n", aux_from);
        }
        if (strcmp(read_part, "To:") == 0) {
            strcpy(to, read_part);
            fgets(to + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "Cc:") == 0 || strcmp(read_part, "CC:") == 0) {
            strcpy(cc, read_part);
            fgets(to + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "Bcc:") == 0) {
            strcpy(bcc, read_part);
            fgets(bcc + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "Subject:") == 0) {
            strcpy(subject, read_part);
            fgets(subject + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "Date:") == 0) {
            strcpy(date, read_part);
            fgets(date + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "Message-id:") == 0 || strcmp(read_part, "Message-ID:") == 0) {
            strcpy(message_id, read_part);
            fgets(message_id + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "Priority:") == 0) {
            strcpy(priority, read_part);
            fgets(priority + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "X-Priority:") == 0) {
            strcpy(x_priority, read_part);
            fgets(x_priority + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "References:") == 0) {
            strcpy(references, read_part);
            fgets(references + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "Newsgroup:") == 0) {
            strcpy(newsgroup, read_part);
            fgets(newsgroup + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "In-Reply-To:") == 0) {
            strcpy(in_reply_to, read_part);
            fgets(in_reply_to + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "Reply-To:") == 0) {
            strcpy(reply_to, read_part);
            fgets(reply_to + strlen(read_part), 100 - strlen(read_part), email);
        }
        if (strcmp(read_part, "Content-Type:") == 0) { 
            if (read_content_type == 0) {
                strcpy(content_type, read_part);
                fgets(content_type + strlen(read_part), 100 - strlen(read_part), email);
                read_content_type = 1;
            }
        }
    }
    fclose(email);
    strncpy(from, aux_from, strlen(aux_from));
    printf("From: %s\n", from);
    printf("To: %s\n", to);
    printf("Cc: %s\n", cc);
    printf("Bcc: %s\n", bcc);
    printf("Subject: %s\n", subject);
    printf("Date: %s\n", date);
    printf("Message-ID: %s\n", message_id);
    printf("Priority: %s X-Priority: %s\n", priority, x_priority);
    printf("References: %s\n", references);
    printf("Newsgroup: %s\n", newsgroup);
    printf("In-Reply-To: %s\n", in_reply_to);
    printf("Reply-To: %s\n", reply_to);
    printf("Content-Type: %s\n", content_type);
    return 0;
}
