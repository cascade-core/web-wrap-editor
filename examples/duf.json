{
    "_": "<?php printf('_%c%c}%c',34,10,10);__halt_compiler();?>",
    "form": {
    },
    "field_groups": {
        "contact": {
            "fields": {
                "from": {
                    "type": "text",
                    "label": "Your name",
                    "default": "Melody",
                    "required": true
                },
                "subject": {
                    "type": "text",
                    "label": "Subject",
                    "default": "The oldest written message in recorded history",
                    "required": true
                },
                "type": {
                    "type": "select",
                    "label": "Type",
                    "options": {
                        "1": "spam",
                        "2": "notification",
                        "3": "question",
                        "4": "warning"
                    },
                    "default": 2,
                    "required": true
                },
                "body": {
                    "type": "textarea",
                    "label": "Message",
                    "default": "Hello sweetie!"
                }
            }
        },
        "submit": {
            "fields": {
                "submit": {
                    "type": "submit",
                    "label": "Send"
                }
            }
        }
    },
    "layout": {
        "#!": "plain_layout",
        "rows": [
            {
                "class": "first",
                "widgets": [
                    {
                        "#!": "@edit",
                        "group_id": "contact",
                        "field_id": "from"
                    }, {
                        "#!": "@edit",
                        "group_id": "contact",
                        "field_id": "subject"
                    }
                ]
            }, {
                "widgets": [
                    {
                        "#!": "@edit",
                        "group_id": "submit",
                        "field_id": "submit"
                    }
                ]
            }
        ]
    }
}

