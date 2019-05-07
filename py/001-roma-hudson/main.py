#!/usr/bin/python
import json


# TODO: move PATH_DATA_JSON to configurable flags
#
# example:
#
# from argparse import ArgumentParser
# parser = ArgumentParser()
# parser.add_argument("-p", "--path", type=str,
#                     dest="path",
#                     help="path to JSON file for parsing"))
# args = parser.parse_args()
# print(args.path)
#
PATH_DATA_JSON = "./data.json"


# TODO: refactor via map and iterators
# TODO: check reader type
def parse(reader, fn):
    doc = json.load(reader)

    # project
    if "project" not in doc:
        return

    doc = doc["project"]

    # project
    #   .properties
    if "properties" not in doc:
        return

    doc = doc["properties"]

    # project
    #   .properties
    #   .hudson.model.ParametersDefinitionProperty
    if "hudson.model.ParametersDefinitionProperty" not in doc:
        return

    doc = doc["hudson.model.ParametersDefinitionProperty"]

    # project
    #   .properties
    #   .hudson.model.ParametersDefinitionProperty
    #   .parameterDefinitions
    if "parameterDefinitions" not in doc:
        return

    doc = doc["parameterDefinitions"]

    # project
    #   .properties
    #   .hudson.model.ParametersDefinitionProperty
    #   .parameterDefinitions
    #   .hudson.model.BooleanParameterDefinition
    if "hudson.model.BooleanParameterDefinition" not in doc:
        return

    els = doc["hudson.model.BooleanParameterDefinition"]

    for el in els:
        name = el.get("name", None)

        if name is not None:
            fn(name)


def main():
    # to print
    def fn_printer(s):
        print("=>", s)

    # to accumulate
    def fn_list_accumulator(ss):
        def fn(s):
            ss.append(s)
        return fn

    # TODO: check if PATH_DATA_JSON is exists and readable
    # 1.
    with open(PATH_DATA_JSON, 'r') as f:
        parse(f, fn_printer)

    # TODO: check if PATH_DATA_JSON is exists and readable
    # 2.
    with open(PATH_DATA_JSON, 'r') as f:
        ss = list()
        parse(f, fn_list_accumulator(ss))
        print(ss)


if __name__ == "__main__":
    main()
