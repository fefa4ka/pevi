#include "lr_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

lr_result_t lr_file_open(struct linked_ring *buffer, char *path, size_t size)
{
    FILE           *file;
    size_t          buffer_size;
    size_t          line_no;
    size_t          line_no_readed;
    char           *symbol;
    char            line_buffer[BUFSIZ];
    struct lr_cell *cells;
    struct lr_cell *owner_cell;

    file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", path);
        return LR_ERROR_UNKNOWN;
    }
    if (size) {
        buffer_size = size;

    } else {
        fseek(file, 0, SEEK_END);  // seek to end of file
        buffer_size = ftell(file); // get current file pointer
        fseek(file, 0, SEEK_SET);  // seek back to beginning of file
    }

    buffer_size += strlen(path);
    // TODO: coeff dependent on size
    buffer_size = buffer_size * 2;

    cells              = malloc(sizeof(struct lr_cell) * buffer_size);
    lr_result_t result = lr_init(buffer, buffer_size, cells);

    // Read filename
    symbol = path;
    while (*symbol) {
        lr_put(buffer, *(symbol++), 0);
    }

    line_no = 1;
    while (fgets(line_buffer, sizeof(line_buffer), file) != NULL) {
        // TODO:
        int len = strlen(line_buffer);
        if (len == sizeof(line_buffer)) {
            printf("Line bigger that buffer, fixme\n");
            exit(0);
        }
        // WARNING: replacing newline
        if (len > 1) {
            line_buffer[len - 1] = '\0';
            lr_put_string(buffer, line_buffer, lr_owner(line_no++));
            line_no_readed = line_no;

        } else {
            line_no++;
        }
    }

    if (line_no != line_no_readed) {
        lr_file_line_insert(buffer, line_no);
    }

    fclose(file);

    return LR_OK;
}

lr_result_t lr_file_rebuild(struct linked_ring *buffer) { return LR_OK; }

lr_result_t lr_file_line_merge(struct linked_ring *buffer, size_t line_no,
                               size_t merged_line_no)
{
    char           *symbol;
    lr_data_t       data;
    struct lr_cell *needle;
    struct lr_cell *merged_line;
    merged_line = lr_owner_find(buffer, lr_owner(merged_line_no));

    for (needle = --merged_line; needle >= buffer->owners; needle--) {
        if (needle->data) {
            // 0 -- filename
            needle->data = lr_owner((size_t)needle->data - 1);
        }
    }


    while (lr_get(buffer, &data, lr_owner(merged_line_no)) == LR_OK) {
        lr_put(buffer, data, lr_owner(line_no));
    }

    return LR_OK;
}

lr_result_t lr_file_line_insert(struct linked_ring *buffer, size_t line_no)
{
    struct lr_cell *line     = NULL;
    struct lr_cell *new_line = NULL;
    new_line                 = lr_owner_allocate(buffer);

    for (line = lr_last_cell(buffer); line >= buffer->owners; line--) {
        if ((size_t)line->data >= line_no) {
            break;
        }
    }

    if ((size_t)line->data == 0) {
        line = line - 1;
    }

    struct lr_cell *needle = NULL;
    for (needle = new_line; needle < line; needle++) {
        *needle = *(needle + 1);
        if (needle->data) {
            // 0 -- filename
            needle->data = lr_owner((size_t)needle->data + 1);
        }
    }

    line->data = lr_owner(line_no);
    line->next = NULL;

    buffer->owners = new_line;

    return LR_OK;
}

lr_result_t lr_file_line_pull(struct linked_ring *buffer, size_t line_no,
                              char *data)
{
    return LR_OK;
}

lr_result_t lr_file_path(struct linked_ring *buffer, char *path)
{
    lr_read_string(buffer, path, 0);
}

lr_result_t lr_file_rename(struct linked_ring *buffer, char *path)
{
    char     *symbol;
    lr_data_t data;

    while (lr_get(buffer, &data, 0) == LR_OK)
        ;
    symbol = path;
    while (*symbol) {
        lr_put(buffer, *(symbol++), 0);
    }

    return LR_OK;
}


lr_result_t lr_file_put(struct linked_ring *buffer, lr_data_t data)
{
    lr_put(buffer, data, (buffer->owners + 1)->data);
    return LR_OK;
}

lr_result_t lr_file_pull(struct linked_ring *buffer, size_t line_no,
                         size_t index, lr_data_t *data)
{

    lr_pull(buffer, data, lr_owner(line_no), index);
    return LR_OK;
}

lr_result_t lr_file_read(struct linked_ring *buffer, size_t line_no,
                         size_t index, lr_data_t *data)
{
    return LR_OK;
}

lr_result_t lr_file_read_line(struct linked_ring *buffer, size_t line_no,
                              char *data)
{
    lr_read_string(buffer, data, lr_owner(line_no));

    return LR_OK;
}

lr_result_t lr_file_write(struct linked_ring *buffer, size_t line_no,
                          size_t index, lr_data_t data)
{

    lr_insert(buffer, (char)data, lr_owner(line_no), index);

    return LR_OK;
}
lr_result_t lr_file_write_string(struct linked_ring *buffer, size_t line_no,
                                 size_t index, char *data)
{
    while (*data) {
        if (*data == '\n') {
            line_no += 1;
            index = 0;
        } else {
            lr_file_write(buffer, line_no, index++, *(data++));
        }
    }

    return LR_OK;
}

lr_result_t lr_file_write_line(struct linked_ring *buffer, size_t line_no,
                               char *data)
{
    size_t index;
    index = 0;
    while (*data) {
        if (*data == '\n') {
            line_no += 1;
            index = 0;
        } else {
            lr_file_write(buffer, line_no, index++, *(data++));
        }
    }

    return LR_OK;
}

lr_result_t lr_file_save(struct linked_ring *buffer, char *path)
{
    FILE           *file;
    char            line_buffer[BUFSIZ];
    struct lr_cell *line;

    file = fopen(path, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", path);
        return LR_ERROR_UNKNOWN;
    }

    for (line = lr_last_cell(buffer); line >= buffer->owners; line--) {
        struct lr_cell *prev_line;
        if ((size_t)line->data > 1) {
            prev_line = line + 1;
            size_t from, to;
            from = (size_t)prev_line->data;
            to   = (size_t)line->data;
            if (to - from > 0)
                for (size_t index = 0; index < to - from - 1; index++)
                    fprintf(file, "\n");
        }


        if ((size_t)line->data) {
            if (line->next) {
                lr_read_string(buffer, line_buffer, lr_owner(line->data));
                fprintf(file, "%s\n", line_buffer);
            }
        }
    }


    fclose(file);

    return LR_OK;
}
