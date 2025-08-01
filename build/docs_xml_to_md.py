import os
import pathlib
import re
import xml.etree.ElementTree as ET
from typing import Dict, List


def to_markdown(text: str):
    text = text.replace("[b]", "`").replace("[/b]", "`")
    text = text.replace("[i]", "`").replace("[/i]", "`")
    text = text.replace("[kbd]", "- ").replace("[/kbd]", "")
    text = text.replace("[code]", "`").replace("[/code]", "`")
    text = text.replace("[code skip-lint]", "`").replace("[/code skip-lint]", "`")

    # Handle code blocks
    start_code_block = text.find("[codeblocks]")
    end_code_block = text.find("[/codeblocks]", start_code_block)

    if start_code_block != -1 and end_code_block != -1:
        code_block_markdown = text[start_code_block:end_code_block]
        code_block_markdown = code_block_markdown.replace("[gdscript]", "```gdscript\n")
        code_block_markdown = code_block_markdown.replace("[gdscript skip-lint]", "```gdscript")
        code_block_markdown = code_block_markdown.replace("[/gdscript]", "```")
        # code_block_markdown = code_block_markdown.replace("[csharp skip-lint]", "```csharp")
        # code_block_markdown = code_block_markdown.replace("[csharp]", "```csharp")
        # code_block_markdown = code_block_markdown.replace("[/csharp]", "```")
        text = text[:start_code_block] + code_block_markdown + text[end_code_block:]

    text = text.replace("[codeblock]", "```gdscript")
    text = text.replace("[/codeblock]", "```")
    text = text.replace("[codeblock lang=text]", "```gdscript")

    text = text.replace("[gdscript]", "```gdscript\n")
    text = text.replace("[gdscript skip-lint]", "```gdscript")
    text = text.replace("[/gdscript]", "```")

    return text


class GodotClassDocumentation:
    def __init__(self, root_xml_element: ET.Element):
        self.root = root_xml_element

        # Class info
        self.class_name = ""
        self.inherits_from = ""
        self.brief_description = ""
        self.full_description = ""

        # Tutorial info
        self.tutorials: List[Dict[str, str]] = []  # [{"title": "name", "link": "https://link.com"}]

        # Signal info
        self.signals: List[Dict[str, str]] = []  # [{"name": "signal_name", "description": "My signal desc"}]

        # Methods info
        self.methods: List[Dict[str, str]] = []

        # Members info
        self.members: List[Dict[str, str]] = []

        # Constants info
        self.constants: List[Dict[str, str]] = []
        self.annotations: List[Dict[str, str]] = []

        self.get_class_info()
        self.get_tutorial_info()
        self.get_signal_info()
        self.get_methods_info()
        self.get_members_info()
        self.get_constants_info()
        self.get_annotations_info()

    def get_default(self, item: str) -> str:
        if item:
            return f", Default - {item}"
        else:
            return ""

    def get_link(self, item: str) -> str:
        docs_url = "https://docs.godotengine.org/en/stable"

        if item:
            return item.replace("$DOCS_URL", docs_url)
        else:
            return ""

    def remove_leading_underscores(self, input_string: str):
        lines = input_string.split("\n")
        out_lines = ""

        ignore = False

        for line in lines:
            if (
                line.__contains__("[codeblock]")
                or line.__contains__("[csharp]")
                or line.__contains__("[csharp skip-lint]")
            ):
                ignore = True
            if line.__contains__("[/codeblock]") or line.__contains__("[/csharp]"):
                ignore = False
                continue

            if ignore:
                continue

            words = line.split()
            modified_words = []

            needs_modification = any(word.startswith("_") for word in words)

            if needs_modification:
                for word in words:
                    if word.startswith("_"):
                        modified_words.append(word.lstrip("_"))
                    else:
                        modified_words.append(word)
                out_lines += "\n" + " ".join(modified_words)
            else:
                out_lines += "\n" + line

        return out_lines

    def generate_markdown(self):
        markdown_content = ""
        markdown_content += f"# {to_markdown(self.class_name)}"
        markdown_content += f"{to_markdown(self.inherits_from)}\n"
        markdown_content += f"{to_markdown(self.brief_description)}\n"
        markdown_content += f"{to_markdown(self.full_description)}\n"

        # Tutorials
        tutorials_md = "\n".join(
            [f"[{to_markdown(item['title'])}]({self.get_link(item['link'])})" for item in self.tutorials]
        )

        # Signals
        signals_md = "\n".join(
            [f"- `{to_markdown(item['name'])}`: {to_markdown(item['description'])}" for item in self.signals]
        )

        # Methods
        methods_md = ""
        for item in self.methods:
            methods_md += (
                f"## {to_markdown(self.class_name)}::{to_markdown(item['name'])} -> {to_markdown(item['return_type'])}"
            )
            methods_md += f"{to_markdown(item['description'])}\n\n"
            if len(item["parameters"]) > 0:
                methods_md += "### Parameters\n"
                for param in item["parameters"]:
                    default_value = f"= {to_markdown(param['default'])}" if param["default"] else ""
                    enum_value = f"- Enum: {to_markdown(param['enum'])}" if param["enum"] else ""
                    methods_md += f"- `{to_markdown(param['name'])}`: {to_markdown(param['type'])} {default_value} {enum_value}\n\n"

        annotations_md = ""
        for item in self.annotations:
            annotations_md += (
                f"## {to_markdown(self.class_name)}::{to_markdown(item['name'])} -> {to_markdown(item['return_type'])}"
            )
            annotations_md += f"{to_markdown(item['description'])}\n\n"
            if len(item["parameters"]) > 0:
                annotations_md += "### Parameters\n"
                for param in item["parameters"]:
                    default_value = f"= {to_markdown(param['default'])}" if param["default"] else ""
                    enum_value = f"- Enum: {to_markdown(param['enum'])}" if param["enum"] else ""
                    annotations_md += f"- `{to_markdown(param['name'])}`: {to_markdown(param['type'])} {default_value} {enum_value}\n\n"

        # Members
        members_md = "\n".join(
            [
                f"- `{to_markdown(item['name'])}`: Type - {to_markdown(item['type'])}{self.get_default(to_markdown(item['default_value']))}"
                for item in self.members
            ]
        )

        # Constants
        constants_md = "\n".join(
            [f"- `{to_markdown(item['name'])}`: Value - {to_markdown(item['value'])}" for item in self.constants]
        )

        # Combine everything into one Markdown string
        markdown_content = f"# {to_markdown(self.class_name)} : public {to_markdown(self.inherits_from)}\n\n## Brief Description\n{to_markdown(self.brief_description)}\n\n## Full Description\n{to_markdown(self.full_description)}\n\n## Tutorials\n{tutorials_md}\n\n## {to_markdown(self.class_name)}.Signals\n{signals_md}\n\n## {to_markdown(self.class_name)}.Methods\n{methods_md}\n\n## {to_markdown(self.class_name)}.Annotations\n{annotations_md}\n\n## {to_markdown(self.class_name)}.Members\n{members_md}\n\n## {to_markdown(self.class_name)}.Constants\n{constants_md}"

        # Trim leading and trailing whitespace from each line
        markdown_content = "\n".join(line for line in markdown_content.split("\n"))

        # Replace double spaces with a single space to prevent extra spacing in rendered Markdown
        markdown_content = markdown_content.replace("  ", " ")
        markdown_content = self.remove_leading_underscores(markdown_content)

        return markdown_content

    def get_class_info(self):
        self.class_name = self.root.attrib["name"]
        try:
            self.inherits_from = self.root.attrib["inherits"]
        except KeyError:
            self.inherits_from = ""

        desc = self.root.find("brief_description")
        if desc is not None and desc.text:
            self.brief_description = desc.text

        desc = self.root.find("description")
        if desc is not None and desc.text:
            self.full_description = desc.text

    def get_tutorial_info(self):
        tutorials = self.root.find("tutorials")
        if tutorials is None:
            return
        for link in tutorials.findall("link"):
            title = link.attrib["title"]
            if link.text:
                url = link.text
                self.tutorials.append({"title": title, "link": url})

    def get_signal_info(self):
        signals = self.root.find("signals")
        if signals is None:
            return
        for signal in signals.findall("signal"):
            name = signal.attrib["name"]
            descriptions = signal.find("description")
            if descriptions is not None and descriptions.text:
                description = descriptions.text
                self.signals.append({"name": name, "description": description})

    def get_methods_info(self):
        methods = self.root.find("methods")

        if methods is None:
            return

        for method in methods.findall("method"):
            method_name = method.attrib["name"]
            return_type_xml = method.find("return")
            return_type = ""
            description = ""

            if return_type_xml is not None:
                return_type = return_type_xml.attrib.get("type", "")

            desc = method.find("description")
            if desc is not None and desc.text:
                description = desc.text

            params = method.findall("param")
            parameters = []
            for param in params:
                index = param.attrib["index"]
                name = param.attrib["name"]
                type_ = param.attrib["type"]
                default_value = param.attrib.get("default", "")
                enum = param.attrib.get("enum", "")

                parameters.append({"index": index, "name": name, "type": type_, "default": default_value, "enum": enum})

            self.methods.append(
                {
                    "name": method_name,
                    "return_type": return_type,
                    "description": description,
                    "parameters": parameters,
                }
            )

    def get_members_info(self):
        members = self.root.find("members")

        if members is None:
            return

        for member in members.findall("member"):
            name = member.attrib["name"]
            type_ = member.attrib["type"]
            setter = member.attrib.get("setter", "")
            getter = member.attrib.get("getter", "")
            enum = member.attrib.get("enum", "")
            default_value = member.attrib.get("default", "")
            description = ""
            if member.text:
                description = member.text

            self.members.append(
                {
                    "name": name,
                    "type": type_,
                    "setter": setter,
                    "getter": getter,
                    "enum": enum,
                    "default_value": default_value,
                    "description": description,
                }
            )

    def get_annotations_info(self):
        annotations = self.root.find("annotations")

        if annotations is None:
            return

        for annotation in annotations.findall("annotation"):
            name = annotation.attrib["name"]
            desc = annotation.find("description")
            description = ""
            if desc is not None and desc.text:
                description = desc.text
            return_type_xml = annotation.find("return")
            return_type = ""

            if return_type_xml is not None:
                return_type = return_type_xml.attrib.get("type", "")

            params = annotation.findall("param")
            parameters = []
            for param in params:
                index = param.attrib["index"]
                name = param.attrib["name"]
                type_ = param.attrib["type"]
                default_value = param.attrib.get("default", "")
                enum = param.attrib.get("enum", "")

                parameters.append({"index": index, "name": name, "type": type_, "default": default_value, "enum": enum})

            self.annotations.append(
                {
                    "name": name,
                    "return_type": return_type,
                    "description": description,
                    "parameters": parameters,
                }
            )

    def get_constants_info(self):
        constants = self.root.find("constants")

        if constants is None:
            return

        for constant in constants.findall("constant"):
            name = constant.attrib["name"]
            value = constant.attrib["value"]
            enum = constant.attrib.get("enum", "")
            description = ""
            if constant.text:
                description = constant.text

            self.constants.append(
                {
                    "name": name,
                    "value": value,
                    "enum": enum,
                    "description": description,
                }
            )


def find_doc_classes_directories(root_path: str) -> List[pathlib.Path]:
    root = pathlib.Path(root_path)
    return [path for path in root.rglob("doc_classes") if path.is_dir()]


def _run(godot_docs: List[str], godot_docs_path: str):
    for doc_class in godot_docs:
        with open(f"{godot_docs_path}/{doc_class}", "r") as file:
            xml_string = file.read()
        root_xml_element = ET.fromstring(xml_string)
        docs = GodotClassDocumentation(root_xml_element)

        markdown = docs.generate_markdown()
        markdown_lines = markdown.splitlines(True)
        markdown_str = ""
        ignore_strip = False

        for line in markdown_lines:
            if line.startswith("\t\t\t"):
                pass

            stripped = line.strip()

            if stripped.__contains__("```"):
                markdown_str += stripped + "\n"
                ignore_strip = not ignore_strip
            elif ignore_strip:
                markdown_str += line
                markdown_str = markdown_str.replace("\t\t\t\t", "")
            else:
                markdown_str += stripped + "\n"

        markdown_str = markdown_str.replace("\n\n\n\n", "\n\n")
        markdown_str = markdown_str.replace("\n\n\n###", "\n\n###")
        markdown_str = markdown_str.replace("\n\n\n##", "\n\n##")
        markdown_str = markdown_str.replace("\n# ", "# ")
        markdown_str = markdown_str.replace("```gdscript\n\n", "```gdscript\n")

        markdown_str = markdown_str.replace("[codeblocks]\n", "")
        markdown_str = markdown_str.replace("[/codeblocks]\n", "")

        brackets_pattern = re.compile(r"\[(signal|enum|param|method|constant|member)\s+([A-Za-z_0-9\./@]*)\]")
        markdown_str = brackets_pattern.sub(r"\2", markdown_str)

        brackets_short_pattern = re.compile(r"\[([A-Za-z_0-9\.]*)\]([^)])")
        markdown_str = brackets_short_pattern.sub(r"\1\2", markdown_str)

        bbcode_url_pattern = re.compile(r"\[url=(.*?)\](.*?)\[\/url\]")
        markdown_str = bbcode_url_pattern.sub(r"[\2](\1)", markdown_str)

        doc_class = doc_class.replace(".xml", "")
        with open(f"{'/home/dm/Documents/gd-md-docs'}/{doc_class}.md", "w") as file:
            file.write(markdown_str)


def run():
    path_to_godot = "/home/dm/dev/godot"
    for path in find_doc_classes_directories(path_to_godot):
        godot_docs = os.listdir(path.__str__())
        godot_docs = [x for x in godot_docs if x.endswith(".xml")]
        _run(godot_docs, path.__str__())

    godot_docs_path = "/home/dm/dev/godot/doc/classes"
    godot_docs = os.listdir(godot_docs_path)

    _run(godot_docs, godot_docs_path)


run()
